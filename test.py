import numpy as np
from numba import cuda
import math
import time


# Kernel CUDA para realizar operaciones intensivas
@cuda.jit
def stressor_kernel(data, iterations):
    idx = cuda.grid(1)  # Obtener el índice global del hilo
    if idx < data.size:
        x = data[idx]
        for _ in range(iterations):
            # Usar las funciones matemáticas del módulo math
            x = (x * x + 2.0) / (x + 1.0) + math.sin(x) ** 2
        data[idx] = x


def gpu_stressor(array_size=10**6, iterations=1000):
    # Crear un array en el host
    data = np.random.rand(array_size).astype(np.float32)

    # Copiar el array a la memoria de la GPU
    d_data = cuda.to_device(data)

    # Configurar el grid y los bloques
    threads_per_block = 256
    blocks_per_grid = (array_size + threads_per_block - 1) // threads_per_block

    print(f"Iniciando el stressor con {array_size} elementos y {iterations} iteraciones...")
    start = time.time()

    # Lanzar el kernel CUDA
    stressor_kernel[blocks_per_grid, threads_per_block](d_data, iterations)

    # Copiar los resultados de vuelta al host
    d_data.copy_to_host(data)
    end = time.time()

    print(f"Stressor completado en {end - start:.2f} segundos.")


if __name__ == "__main__":
    gpu_stressor(array_size=10**7, iterations=2000)
