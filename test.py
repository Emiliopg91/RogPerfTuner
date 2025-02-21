def agrupar_consecutivos(lista):
    grupos = []
    inicio = lista[0]

    for i in range(1, len(lista)):
        if lista[i] != lista[i - 1] + 1:
            if inicio == lista[i - 1]:
                grupos.append(f"{inicio}")
            else:
                grupos.append(f"{inicio}-{lista[i - 1]}")
            inicio = lista[i]

    # Añadir el último grupo
    if inicio == lista[-1]:
        grupos.append(f"{inicio}")
    else:
        grupos.append(f"{inicio}-{lista[-1]}")

    return grupos


# Ejemplo de uso
lista = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
resultado = agrupar_consecutivos(lista)
print(resultado)
lista = [0, 1, 2, 4, 5, 6, 8, 9, 10]
resultado = agrupar_consecutivos(lista)
print(resultado)
