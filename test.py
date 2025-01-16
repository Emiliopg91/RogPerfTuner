def create_anti_clockwise_border_matrix(size, numbers):
    # Crear una matriz de ceros del tamaño indicado
    matrix = [[" "] * size for _ in range(size)]

    # Coordenadas iniciales
    top, bottom, left, right = 0, size - 1, 0, size - 1

    index = 0  # Índice para recorrer los números

    # Rellenar bordes en sentido antihorario
    while index < len(numbers) and (top <= bottom and left <= right):
        # Borde superior: izquierda a derecha
        for col in range(left, right + 1):
            if index < len(numbers):
                matrix[top][col] = numbers[index]
                index += 1
        top += 1

        # Borde derecho: arriba hacia abajo
        for row in range(top, bottom + 1):
            if index < len(numbers):
                matrix[row][right] = numbers[index]
                index += 1
        right -= 1

        # Borde inferior: derecha a izquierda
        for col in range(right, left - 1, -1):
            if index < len(numbers):
                matrix[bottom][col] = numbers[index]
                index += 1
        bottom -= 1

        # Borde izquierdo: abajo hacia arriba
        for row in range(bottom, top - 1, -1):
            if index < len(numbers):
                matrix[row][left] = numbers[index]
                index += 1
        left += 1

    return matrix


# Ejemplo de uso
numbers = list(range(25))  # Números del 0 al 14
size = 8  # Tamaño de la matriz (5x5)
result = create_anti_clockwise_border_matrix(size, numbers)

# Imprimir la matriz
for row in result:
    print(row)
