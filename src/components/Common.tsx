import { ReactNode } from "react";
import {
    Box,
    Button,
    Dialog,
    DialogActions,
    DialogContent,
    DialogContentText,
    DialogTitle,
    MenuItem,
    Paper,
    Select,
    Typography,
} from "@mui/material";

import { useTranslate } from "@/context/I18nContext";

/// Titled group of settings (QGroupBox)
export function Section({ title, children }: { title: string; children: ReactNode }) {
    return (
        <Paper variant="outlined" sx={{ p: 2, display: "flex", flexDirection: "column", gap: 1.5 }}>
            <Typography variant="subtitle2" color="primary">
                {title}
            </Typography>
            {children}
        </Paper>
    );
}

/// Label + control row (QFormLayout row)
export function Row({ label, children }: { label: string; children: ReactNode }) {
    return (
        <Box sx={{ display: "grid", gridTemplateColumns: "40% 60%", alignItems: "center", gap: 1 }}>
            <Typography variant="body2">{label}:</Typography>
            <Box sx={{ display: "flex", gap: 1, alignItems: "center", minWidth: 0 }}>{children}</Box>
        </Box>
    );
}

export interface Option<T extends string | number> {
    value: T;
    label: string;
}

/// Dropdown bound to a list of options
export function OptionSelect<T extends string | number>({
    value,
    options,
    disabled,
    onChange,
}: {
    value: T;
    options: Option<T>[];
    disabled?: boolean;
    onChange: (value: T) => void;
}) {
    return (
        <Select
            value={options.some((o) => o.value === value) ? value : ""}
            disabled={disabled}
            onChange={(e) => {
                const raw = e.target.value as T;
                const opt = options.find((o) => String(o.value) === String(raw));
                if (opt) {
                    onChange(opt.value);
                }
            }}
        >
            {options.map((o) => (
                <MenuItem key={String(o.value)} value={o.value}>
                    {o.label}
                </MenuItem>
            ))}
        </Select>
    );
}

/// Yes/no confirmation (YesNoDialog)
export function ConfirmDialog({
    open,
    title,
    body,
    onAnswer,
}: {
    open: boolean;
    title: string;
    body: string;
    onAnswer: (accepted: boolean) => void;
}) {
    const t = useTranslate();
    return (
        <Dialog open={open} onClose={() => onAnswer(false)}>
            <DialogTitle>{title}</DialogTitle>
            <DialogContent>
                <DialogContentText>{body}</DialogContentText>
            </DialogContent>
            <DialogActions>
                <Button onClick={() => onAnswer(false)}>{t("cancel")}</Button>
                <Button variant="contained" onClick={() => onAnswer(true)} autoFocus>
                    {t("accept")}
                </Button>
            </DialogActions>
        </Dialog>
    );
}

export function capitalize(s: string): string {
    return s.length ? s[0].toUpperCase() + s.slice(1).toLowerCase() : s;
}
