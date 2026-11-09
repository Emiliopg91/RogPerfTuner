import { useEffect, useRef, useState } from "react";
import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, Tab, Tabs } from "@mui/material";

import { BackendClient } from "@/utils/backend/client";
import { FanCurveData, FanCurves } from "@/utils/backend/models";
import { useTranslate } from "@/context/I18nContext";

/// profile -> fan -> curve
type CurveMap = Record<string, Record<string, FanCurveData>>;

const PROFILE_ORDER = ["performance", "balanced", "quiet"];

function cloneCurves(src: FanCurves, pick: "current" | "defaults"): CurveMap {
    const out: CurveMap = {};
    for (const [profile, fans] of Object.entries(src.curves)) {
        out[profile] = {};
        for (const [fan, set] of Object.entries(fans)) {
            out[profile][fan] = { temp: [...set[pick].temp], perc: [...set[pick].perc] };
        }
    }
    return out;
}

const W = 460;
const H = 320;
const PAD = { left: 44, right: 14, top: 14, bottom: 34 };
const toX = (temp: number) => PAD.left + (temp / 100) * (W - PAD.left - PAD.right);
const toY = (perc: number) => H - PAD.bottom - (perc / 100) * (H - PAD.top - PAD.bottom);

/// Draggable curve chart: each point is limited by its neighbours.
function CurveChart({ curve, onChange }: { curve: FanCurveData; onChange: (curve: FanCurveData) => void }) {
    const svgRef = useRef<SVGSVGElement>(null);
    const [selected, setSelected] = useState<number | null>(null);

    const toValue = (e: React.PointerEvent) => {
        const rect = svgRef.current!.getBoundingClientRect();
        const x = ((e.clientX - rect.left) / rect.width) * W;
        const y = ((e.clientY - rect.top) / rect.height) * H;
        return {
            temp: ((x - PAD.left) / (W - PAD.left - PAD.right)) * 100,
            perc: ((H - PAD.bottom - y) / (H - PAD.top - PAD.bottom)) * 100,
        };
    };

    const closest = (temp: number, perc: number) => {
        let best = -1;
        let bestDist = Infinity;
        curve.temp.forEach((t, i) => {
            const d = Math.hypot(t - temp, curve.perc[i] - perc);
            if (d < bestDist) {
                bestDist = d;
                best = i;
            }
        });
        return best;
    };

    const onMove = (e: React.PointerEvent) => {
        if (selected === null) {
            return;
        }
        const { temp, perc } = toValue(e);
        const n = curve.temp.length;
        const minX = selected === 0 ? 0 : curve.temp[selected - 1];
        const maxX = selected === n - 1 ? 100 : curve.temp[selected + 1];
        const minY = selected === 0 ? 0 : curve.perc[selected - 1];
        const maxY = selected === n - 1 ? 100 : curve.perc[selected + 1];
        const next = { temp: [...curve.temp], perc: [...curve.perc] };
        next.temp[selected] = Math.round(Math.min(maxX, Math.max(minX, temp)));
        next.perc[selected] = Math.round(Math.min(maxY, Math.max(minY, perc)));
        onChange(next);
    };

    const ticks = [0, 20, 40, 60, 80, 100];
    const path = curve.temp.map((t, i) => `${i === 0 ? "M" : "L"}${toX(t)},${toY(curve.perc[i])}`).join(" ");

    return (
        <Box
            component="svg"
            ref={svgRef}
            viewBox={`0 0 ${W} ${H}`}
            sx={{ width: "100%", touchAction: "none", userSelect: "none", cursor: selected !== null ? "grabbing" : "default" }}
            onPointerDown={(e: React.PointerEvent<SVGSVGElement>) => {
                const { temp, perc } = toValue(e);
                const idx = closest(temp, perc);
                if (idx >= 0) {
                    setSelected(idx);
                    e.currentTarget.setPointerCapture(e.pointerId);
                }
            }}
            onPointerMove={onMove}
            onPointerUp={() => setSelected(null)}
        >
            {ticks.map((v) => (
                <g key={v}>
                    <line x1={toX(v)} x2={toX(v)} y1={toY(0)} y2={toY(100)} stroke="#444" strokeWidth={0.5} />
                    <line x1={toX(0)} x2={toX(100)} y1={toY(v)} y2={toY(v)} stroke="#444" strokeWidth={0.5} />
                    <text x={toX(v)} y={H - 12} fill="#aaa" fontSize={11} textAnchor="middle">
                        {v}c
                    </text>
                    <text x={PAD.left - 6} y={toY(v) + 4} fill="#aaa" fontSize={11} textAnchor="end">
                        {v}%
                    </text>
                </g>
            ))}
            <path d={path} fill="none" stroke="#d6001c" strokeWidth={2} />
            {curve.temp.map((t, i) => (
                <g key={i}>
                    <circle cx={toX(t)} cy={toY(curve.perc[i])} r={selected === i ? 7 : 5} fill="#d6001c" />
                    {selected === i && (
                        <text x={toX(t)} y={toY(curve.perc[i]) - 12} fill="#fff" fontSize={11} textAnchor="middle">
                            {t}c / {curve.perc[i]}%
                        </text>
                    )}
                </g>
            ))}
        </Box>
    );
}

/// Fan curve editor
export default function FanCurveDialog({ onClose }: { onClose: () => void }) {
    const t = useTranslate();
    const [data, setData] = useState<FanCurves | null>(null);
    const [curves, setCurves] = useState<CurveMap>({});
    const [profile, setProfile] = useState("performance");
    const [fan, setFan] = useState("");
    const [saving, setSaving] = useState(false);

    useEffect(() => {
        BackendClient.getFanCurves().then((d) => {
            setData(d);
            setCurves(cloneCurves(d, "current"));
            setProfile(PROFILE_ORDER.includes(d.selected_profile) ? d.selected_profile : "performance");
            setFan(d.fans[0] ?? "");
        });
    }, []);

    if (!data) {
        return null;
    }

    const save = async () => {
        setSaving(true);
        await BackendClient.saveFanCurves(curves);
        // "Reset" goes back to what was just saved
        setData(await BackendClient.getFanCurves());
        setSaving(false);
    };

    const curve = curves[profile]?.[fan];

    return (
        <Dialog open onClose={onClose} fullWidth maxWidth="sm">
            <DialogTitle>{t("edit.fan.profile")}</DialogTitle>
            <DialogContent>
                <Tabs value={profile} onChange={(_, v) => setProfile(v)} variant="fullWidth">
                    {PROFILE_ORDER.filter((p) => curves[p]).map((p) => (
                        <Tab key={p} value={p} label={t(`label.profile.${p.toUpperCase()}`)} />
                    ))}
                </Tabs>
                <Tabs value={fan} onChange={(_, v) => setFan(v)} variant="fullWidth" sx={{ mb: 1 }}>
                    {data.fans.map((f) => (
                        <Tab key={f} value={f} label={f} />
                    ))}
                </Tabs>
                {curve && (
                    <CurveChart
                        curve={curve}
                        onChange={(c) => setCurves({ ...curves, [profile]: { ...curves[profile], [fan]: c } })}
                    />
                )}
            </DialogContent>
            <DialogActions sx={{ flexWrap: "wrap", gap: 1 }}>
                <Button onClick={() => setCurves(cloneCurves(data, "current"))}>{t("reset")}</Button>
                <Button onClick={() => setCurves(cloneCurves(data, "defaults"))}>{t("reset.default")}</Button>
                <Box sx={{ flexGrow: 1 }} />
                <Button onClick={onClose}>{t("close")}</Button>
                <Button variant="contained" disabled={saving} onClick={save}>
                    {t("save")}
                </Button>
            </DialogActions>
        </Dialog>
    );
}
