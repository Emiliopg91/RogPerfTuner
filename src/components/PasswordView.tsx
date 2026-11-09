import { useState } from "react";
import { Alert, Box, Button, TextField, Typography } from "@mui/material";

import { BackendClient } from "@/utils/backend/client";
import { useTranslate } from "@/context/I18nContext";

/// Asks for the sudo password on first run (replaces `PasswordDialog`).
export default function PasswordView() {
    const t = useTranslate();
    const [password, setPassword] = useState("");
    const [failed, setFailed] = useState(false);
    const [busy, setBusy] = useState(false);

    const submit = async () => {
        setBusy(true);
        const ok = await BackendClient.submitPassword(password);
        setBusy(false);
        if (!ok) {
            setFailed(true);
            setPassword("");
        }
    };

    return (
        <Box sx={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 2, p: 4, pt: 8 }}>
            <img src="/icon.svg" width={120} alt="" />
            <Typography variant="h6">{t("authentication.required")}</Typography>
            {failed && <Alert severity="error">{t("authentication.failed")}</Alert>}
            <TextField
                type="password"
                autoFocus
                label={t("enter.sudo.password")}
                value={password}
                disabled={busy}
                onChange={(e) => setPassword(e.target.value)}
                onKeyDown={(e) => e.key === "Enter" && password && submit()}
            />
            <Box sx={{ display: "flex", gap: 1 }}>
                <Button variant="contained" disabled={!password || busy} onClick={submit}>
                    {t("accept")}
                </Button>
                <Button onClick={() => BackendClient.quit()}>{t("cancel")}</Button>
            </Box>
        </Box>
    );
}
