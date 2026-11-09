import { useCallback, useEffect, useRef, useState } from "react";
import { Box, Button, Checkbox, FormControlLabel, Typography } from "@mui/material";

import { BackendClient } from "@/utils/backend/client";
import { BackendListener } from "@/utils/backend/listener";
import { AppState, OpenGameConfig } from "@/utils/backend/models";
import { useTranslate } from "@/context/I18nContext";
import { ConfirmDialog, OptionSelect, Row, Section, capitalize } from "@/components/Common";
import GamesDialog from "@/components/GamesDialog";
import GameConfigDialog from "@/components/GameConfigDialog";
import FanCurveDialog from "@/components/FanCurveDialog";

/// Main window.
export default function MainView({ appName, version }: { appName: string; version: string }) {
    const t = useTranslate();
    const [state, setState] = useState<AppState | null>(null);
    const [view, setView] = useState<string | null>(null);
    const [gameConfig, setGameConfig] = useState<OpenGameConfig | null>(null);
    const [askDecky, setAskDecky] = useState(false);
    const colorTimer = useRef<ReturnType<typeof setTimeout> | null>(null);

    const reload = useCallback(() => {
        BackendClient.getState().then(setState);
    }, []);

    useEffect(() => {
        reload();
        const unlisten = [
            BackendListener.onStateChanged(reload),
            BackendListener.onShowView(setView),
            BackendListener.onOpenGameConfig(setGameConfig),
            BackendListener.onAskDeckyInstall(() => setAskDecky(true)),
        ];
        return () => unlisten.forEach((u) => u());
    }, [reload]);

    if (!state) {
        return null;
    }

    const gaming = state.running_games > 0;
    // Optimistic update so selectors react immediately; the backend confirms with `state_changed`
    const update = (patch: Partial<AppState>, action: Promise<void>) => {
        setState({ ...state, ...patch });
        action.catch(console.error).finally(reload);
    };

    // The color picker fires continuously while dragging: apply only the last value
    const changeColor = (color: string) => {
        setState({ ...state, color });
        if (colorTimer.current) {
            clearTimeout(colorTimer.current);
        }
        colorTimer.current = setTimeout(() => {
            BackendClient.setColor(color).catch(console.error).finally(reload);
        }, 300);
    };

    return (
        <Box sx={{ display: "flex", flexDirection: "column", gap: 2, p: 2 }}>
            <Box sx={{ textAlign: "center" }}>
                <img src="/icon.svg" width={110} alt={appName} />
                <Typography variant="caption" color="text.secondary" sx={{ display: "block" }}>
                    {appName} {version}
                </Typography>
            </Box>

            <Section title={t("performance")}>
                <Row label={t("profile")}>
                    <OptionSelect
                        value={state.profile}
                        disabled={gaming || state.on_battery}
                        options={state.profiles.map((p) => ({
                            value: p,
                            label: t(`label.profile.${p.toUpperCase()}`),
                        }))}
                        onChange={(profile) => update({ profile }, BackendClient.setProfile(profile))}
                    />
                </Row>
                <Row label={t("scheduler")}>
                    <OptionSelect
                        value={state.scheduler}
                        disabled={gaming}
                        options={state.schedulers.map((s) => ({ value: s, label: s }))}
                        onChange={(scheduler) => update({ scheduler }, BackendClient.setScheduler(scheduler))}
                    />
                </Row>
                {state.ssd_schedulers.length > 0 && (
                    <Row label={t("ssd.scheduler")}>
                        <OptionSelect
                            value={state.ssd_scheduler ?? "none"}
                            options={state.ssd_schedulers.map((s) => ({
                                value: s,
                                label: capitalize(s === "none" ? t("default") : s),
                            }))}
                            onChange={(s) => update({ ssd_scheduler: s }, BackendClient.setSsdScheduler(s))}
                        />
                    </Row>
                )}
                {state.fan_control && (
                    <Row label={t("fan.curves")}>
                        <Button fullWidth onClick={() => setView("fans")}>
                            {t("edit.curve")}
                        </Button>
                    </Row>
                )}
                <Row label={t("games")}>
                    <Button fullWidth onClick={() => setView("games")}>
                        {t("label.game.configure")}
                    </Button>
                </Row>
            </Section>

            <Section title="Aura">
                <Row label={t("effect")}>
                    <OptionSelect
                        value={state.effect}
                        options={state.effects.map((e) => ({ value: e, label: e }))}
                        onChange={(effect) => update({ effect }, BackendClient.setEffect(effect))}
                    />
                </Row>
                <Row label={t("brightness")}>
                    <OptionSelect
                        value={state.brightness}
                        options={state.brightnesses.map((b) => ({
                            value: b.value,
                            label: t(`label.brightness.${b.name}`),
                        }))}
                        onChange={(brightness) => update({ brightness }, BackendClient.setBrightness(brightness))}
                    />
                    <Box
                        component="input"
                        type="color"
                        title={t("pick.color")}
                        disabled={!state.color}
                        value={state.color ?? "#000000"}
                        onChange={(e: React.ChangeEvent<HTMLInputElement>) => changeColor(e.target.value)}
                        sx={{
                            width: 32,
                            height: 32,
                            flexShrink: 0,
                            border: "none",
                            background: "none",
                            cursor: state.color ? "pointer" : "default",
                            opacity: state.color ? 1 : 0.3,
                        }}
                    />
                </Row>
            </Section>

            {(state.charge_limit !== null || state.boot_sound !== null) && (
                <Section title={t("hardware")}>
                    {state.charge_limit !== null && (
                        <Row label={t("charge.threshold")}>
                            <OptionSelect
                                value={state.charge_limit}
                                options={state.charge_limits.map((l) => ({ value: l, label: `${l}%` }))}
                                onChange={(l) => update({ charge_limit: l }, BackendClient.setChargeLimit(l))}
                            />
                        </Row>
                    )}
                    {state.boot_sound !== null && (
                        <Row label={t("boot.sound")}>
                            <OptionSelect
                                value={state.boot_sound ? "true" : "false"}
                                options={[
                                    { value: "true", label: t("enabled") },
                                    { value: "false", label: t("disabled") },
                                ]}
                                onChange={(v) =>
                                    update({ boot_sound: v === "true" }, BackendClient.setBootSound(v === "true"))
                                }
                            />
                        </Row>
                    )}
                </Section>
            )}

            <Section title={t("settings")}>
                <FormControlLabel
                    control={
                        <Checkbox
                            checked={state.start_minimized}
                            onChange={(e) =>
                                update(
                                    { start_minimized: e.target.checked },
                                    BackendClient.setStartMinimized(e.target.checked),
                                )
                            }
                        />
                    }
                    label={t("start.minimized")}
                />
                <FormControlLabel
                    control={
                        <Checkbox
                            checked={state.autostart}
                            disabled={state.dev_mode}
                            onChange={(e) =>
                                update({ autostart: e.target.checked }, BackendClient.setAutostart(e.target.checked))
                            }
                        />
                    }
                    label={t("autostart")}
                />
            </Section>

            <GamesDialog
                open={view === "games"}
                onClose={() => setView(null)}
                onEdit={(id) => setGameConfig({ id, first_run: false })}
            />
            {gameConfig && (
                <GameConfigDialog
                    id={gameConfig.id}
                    firstRun={gameConfig.first_run}
                    onClose={() => setGameConfig(null)}
                />
            )}
            {view === "fans" && <FanCurveDialog onClose={() => setView(null)} />}
            <ConfirmDialog
                open={askDecky}
                title={t("enable.decky.integration.title")}
                body={t("enable.decky.integration.body")}
                onAnswer={(accept) => {
                    setAskDecky(false);
                    BackendClient.answerDeckyInstall(accept);
                }}
            />
        </Box>
    );
}
