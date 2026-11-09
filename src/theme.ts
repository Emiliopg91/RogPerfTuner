import { createTheme } from "@mui/material/styles";

export const theme = createTheme({
    palette: {
        mode: "dark",
        primary: {
            main: "#d6001c",
        },
        background: {
            default: "#1e1e1e",
            paper: "#121212",
        },
    },
    components: {
        MuiSelect: {
            defaultProps: { size: "small", fullWidth: true },
        },
        MuiTextField: {
            defaultProps: { size: "small", fullWidth: true },
        },
        MuiButton: {
            defaultProps: { variant: "outlined", size: "small" },
        },
    },
});
