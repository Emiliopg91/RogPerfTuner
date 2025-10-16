#!/usr/bin/env bash
set -e

# --- Función de ayuda ---
usage() {
    echo "Uso: $0 <ruta_del_submodulo>"
    echo "Ejemplo: $0 external/imgui"
    exit 1
}

# --- Validar argumento ---
if [ $# -ne 1 ]; then
    usage
fi

SUBMODULE_PATH="$1"

# --- Verificar que sea un submódulo ---
if ! git config --file .gitmodules --name-only --get-regexp path | grep -q "$SUBMODULE_PATH"; then
    echo "❌ Error: '$SUBMODULE_PATH' no es un submódulo registrado."
    exit 1
fi

echo "🧹 Eliminando submódulo: $SUBMODULE_PATH"

# 1. Quitar la entrada del submódulo en .gitmodules
git config --file .gitmodules --remove-section "submodule.$SUBMODULE_PATH" 2>/dev/null || true

# 2. Quitar la entrada del submódulo en .git/config
git config --remove-section "submodule.$SUBMODULE_PATH" 2>/dev/null || true

# 3. Eliminar el submódulo del índice
git rm -f "$SUBMODULE_PATH"

# 4. Eliminar el directorio del submódulo
rm -rf ".git/modules/$SUBMODULE_PATH"
rm -rf "$SUBMODULE_PATH"

# 5. Limpiar cambios residuales
git add .gitmodules || true

echo "✅ Submódulo eliminado correctamente: $SUBMODULE_PATH"
echo "💡 Recuerda hacer commit:"
echo "   git commit -m 'Remove submodule $SUBMODULE_PATH'"
