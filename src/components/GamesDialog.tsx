import { useCallback, useEffect, useState } from "react";
import {
    Avatar,
    Button,
    Dialog,
    DialogContent,
    DialogTitle,
    IconButton,
    Table,
    TableBody,
    TableCell,
    TableHead,
    TableRow,
    Tooltip,
    Typography,
} from "@mui/material";
import CloseIcon from "@mui/icons-material/Close";
import SportsEsportsIcon from "@mui/icons-material/SportsEsports";

import { BackendClient } from "@/utils/backend/client";
import { BackendListener } from "@/utils/backend/listener";
import { GameListItem } from "@/utils/backend/models";
import { useTranslate } from "@/context/I18nContext";

/// Configured games list
export default function GamesDialog({
    open,
    onClose,
    onEdit,
}: {
    open: boolean;
    onClose: () => void;
    onEdit: (id: number) => void;
}) {
    const t = useTranslate();
    const [games, setGames] = useState<GameListItem[]>([]);

    const reload = useCallback(() => {
        BackendClient.getGames().then(setGames);
    }, []);

    useEffect(() => {
        if (open) {
            reload();
        }
        return BackendListener.onGamesChanged(reload);
    }, [open, reload]);

    return (
        <Dialog open={open} onClose={onClose} fullWidth maxWidth="sm">
            <DialogTitle sx={{ pr: 6 }}>
                {t("game.performance.configuration")}
                <IconButton onClick={onClose} sx={{ position: "absolute", right: 8, top: 8 }}>
                    <CloseIcon />
                </IconButton>
            </DialogTitle>
            <DialogContent dividers sx={{ p: 0 }}>
                <Table size="small" stickyHeader>
                    <TableHead>
                        <TableRow>
                            <TableCell>{t("game.title")}</TableCell>
                            <TableCell align="right">{t("configuration")}</TableCell>
                        </TableRow>
                    </TableHead>
                    <TableBody>
                        {games.map((g) => (
                            <TableRow key={g.id}>
                                <TableCell>
                                    <Tooltip title={String(g.id)} placement="bottom-start">
                                        <Typography
                                            component="span"
                                            sx={{
                                                display: "flex",
                                                alignItems: "center",
                                                gap: 1,
                                                color: g.running ? "success.main" : undefined,
                                            }}
                                        >
                                            <Avatar src={g.icon ?? undefined} variant="rounded" sx={{ width: 24, height: 24 }}>
                                                <SportsEsportsIcon fontSize="small" />
                                            </Avatar>
                                            {g.name}
                                            {g.running && ` (${t("running")}...)`}
                                        </Typography>
                                    </Tooltip>
                                </TableCell>
                                <TableCell align="right">
                                    <Button onClick={() => onEdit(g.id)}>{t("edit")}</Button>
                                </TableCell>
                            </TableRow>
                        ))}
                    </TableBody>
                </Table>
            </DialogContent>
        </Dialog>
    );
}
