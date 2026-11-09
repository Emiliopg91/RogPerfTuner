import { useEffect, useState } from "react";
import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, TextField } from "@mui/material";

import { BackendClient } from "@/utils/backend/client";
import { GameConfigDetails, GameConfigEntry, GameSuggestion } from "@/utils/backend/models";
import { useTranslate } from "@/context/I18nContext";
import { ConfirmDialog, OptionSelect, Row, Section, capitalize } from "@/components/Common";

/// Per game configuration. On first run the game is launched afterwards.
export default function GameConfigDialog({
    id,
    firstRun,
    onClose,
}: {
    id: number;
    firstRun: boolean;
    onClose: () => void;
}) {
    const t = useTranslate();
    const [details, setDetails] = useState<GameConfigDetails | null>(null);
    const [entry, setEntry] = useState<GameConfigEntry | null>(null);
    const [suggestion, setSuggestion] = useState<GameSuggestion | null>(null);
    const [confirmClose, setConfirmClose] = useState(false);

    useEffect(() => {
        BackendClient.getGameConfig(id).then((d) => {
            setDetails(d);
            setEntry(d.entry);
        });
        if (firstRun) {
            BackendClient.getGameSuggestion(id)
                .then(setSuggestion)
                .catch((e) => console.error("Error while fetching suggestions", e));
        }
    }, [id, firstRun]);

    if (!details || !entry) {
        return null;
    }

    const set = (patch: Partial<GameConfigEntry>) => setEntry({ ...entry, ...patch });

    const save = async () => {
        await BackendClient.saveGameConfig(id, entry, firstRun);
        onClose();
    };

    const requestClose = () => {
        if (firstRun) {
            setConfirmClose(true);
        } else {
            onClose();
        }
    };

    return (
        <Dialog open onClose={requestClose} fullWidth maxWidth="xs">
            <DialogTitle>{entry.name}</DialogTitle>
            <DialogContent sx={{ display: "flex", flexDirection: "column", gap: 2 }}>
                {details.banner && (
                    <Box component="img" src={details.banner} alt="" sx={{ width: "100%", borderRadius: 1 }} />
                )}

                <Section title={t("performance")}>
                    <Row label={t("used.gpu")}>
                        <OptionSelect
                            value={entry.gpu ?? ""}
                            options={[
                                { value: "", label: t("label.dgpu.auto") },
                                ...details.gpus.map((g) => ({ value: g, label: capitalize(g) })),
                            ]}
                            onChange={(gpu) => set({ gpu: gpu || null })}
                        />
                    </Row>
                    <Row label={t("scheduler")}>
                        <OptionSelect
                            value={entry.scheduler ?? details.default_scheduler}
                            disabled={details.schedulers.length === 0}
                            options={details.schedulers.map((s) => ({ value: s, label: capitalize(s) }))}
                            onChange={(scheduler) => set({ scheduler })}
                        />
                    </Row>
                    <Row label={t("metrics")}>
                        <OptionSelect
                            value={entry.metrics_level}
                            disabled={!details.metrics_enabled}
                            options={details.metrics_levels.map((l) => ({ value: l, label: t(`label.level.${l}`) }))}
                            onChange={(metrics_level) => set({ metrics_level })}
                        />
                    </Row>
                </Section>

                {entry.proton && (
                    <Section title="Proton">
                        <Row label={t("used.device")}>
                            <OptionSelect
                                value={entry.device}
                                options={details.devices.map((d) => ({ value: d, label: t(`label.device.${d}`) }))}
                                onChange={(device) => set({ device })}
                            />
                        </Row>
                        <Row label={t("winesync")}>
                            <OptionSelect
                                value={entry.sync}
                                options={details.sync_options.map((s) => ({ value: s, label: t(`label.winesync.${s}`) }))}
                                onChange={(sync) => set({ sync })}
                            />
                        </Row>
                    </Section>
                )}

                <Section title={t("execution")}>
                    <TextField
                        label={t("environment")}
                        value={entry.env ?? ""}
                        onChange={(e) => set({ env: e.target.value })}
                    />
                    <TextField
                        label={t("wrappers")}
                        value={entry.wrappers ?? ""}
                        onChange={(e) => set({ wrappers: e.target.value })}
                    />
                    <TextField
                        label={t("params")}
                        value={entry.args ?? ""}
                        onChange={(e) => set({ args: e.target.value })}
                    />
                </Section>
            </DialogContent>
            <DialogActions>
                <Button variant="contained" fullWidth onClick={save}>
                    {t(firstRun ? "save.and.run" : "save")}
                </Button>
            </DialogActions>

            <ConfirmDialog
                open={suggestion !== null}
                title={t("suggested.game.cfg.found")}
                body={t("apply.suggested.game.cfg")}
                onAnswer={(accepted) => {
                    if (accepted && suggestion) {
                        set({
                            args: suggestion.args ?? entry.args,
                            env: suggestion.env ?? entry.env,
                        });
                    }
                    setSuggestion(null);
                }}
            />
            <ConfirmDialog
                open={confirmClose}
                title={t("confirmation.required")}
                body={t("run.with.default.config")}
                onAnswer={(accepted) => {
                    setConfirmClose(false);
                    if (accepted) {
                        BackendClient.launchGame(id);
                        onClose();
                    }
                }}
            />
        </Dialog>
    );
}
