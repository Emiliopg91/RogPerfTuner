import { useEffect, useState } from "react";
import { Box, Button, CircularProgress, Typography } from "@mui/material";

import { BackendClient } from "@/utils/backend/client";
import { BackendListener } from "@/utils/backend/listener";
import { Bootstrap, InitStatus } from "@/utils/backend/models";
import { I18nContext, buildTranslator } from "@/context/I18nContext";
import PasswordView from "@/components/PasswordView";
import MainView from "@/components/MainView";

/// Root component: waits for the backend initialization and shows the matching view.
export default function App() {
    const [bootstrap, setBootstrap] = useState<Bootstrap | null>(null);
    const [status, setStatus] = useState<InitStatus>(InitStatus.Loading);

    useEffect(() => {
        BackendClient.getBootstrap().then((b) => {
            setBootstrap(b);
            setStatus(b.status);
            document.documentElement.lang = b.language;
        });
        return BackendListener.onStatusChanged(setStatus);
    }, []);

    if (!bootstrap) {
        return null;
    }

    const t = buildTranslator(bootstrap.translations);
    let content;
    switch (status) {
        case InitStatus.Password:
            content = <PasswordView />;
            break;
        case InitStatus.MissingModule:
            content = (
                <Box sx={{ p: 4, textAlign: "center" }}>
                    <Typography variant="h6" gutterBottom>
                        Missing module
                    </Typography>
                    <Typography gutterBottom>Required Asus-Armoury module not loaded in kernel.</Typography>
                    <Button onClick={() => BackendClient.quit()}>{t("close")}</Button>
                </Box>
            );
            break;
        case InitStatus.Ready:
            content = <MainView appName={bootstrap.app_name} version={bootstrap.version} />;
            break;
        default:
            content = (
                <Box sx={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 2, pt: 12 }}>
                    <img src="/icon.svg" width={120} alt="" />
                    <CircularProgress />
                    <Typography>{t("initializing")}</Typography>
                </Box>
            );
    }

    return <I18nContext.Provider value={t}>{content}</I18nContext.Provider>;
}
