import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors

FILENAME = "brains.dat"
VERT_GRAPHS = 5

def read_matrix(fileobj):
    dim = np.fromfile(fileobj, dtype=np.int32, count=2)
    matrix = np.fromfile(fileobj, dtype=np.float32, count=dim[0]*dim[1])
    return matrix.reshape(dim[0], dim[1])


def read_model(fileobj):
    numLayers = np.fromfile(fileobj, dtype=np.int32, count=1)[0]
    weights = []
    biases = []
    for i in range(numLayers - 1):
        weights.append(read_matrix(fileobj))
        biases.append(read_matrix(fileobj))
    return weights, biases

def plot_matrix(matrix):
    plt.imshow(matrix)
    plt.colorbar()
    plt.show()

fileobj = open(FILENAME, mode="rb") #read binary
data = np.fromfile(fileobj, dtype=np.int32, count=2)
numBrains = data[0]
print("Reading model with " + str(numBrains) + " brains of size " + str(data[1]) + " bytes.")
fig, ax = plt.subplots(VERT_GRAPHS, int(numBrains/VERT_GRAPHS), sharex='col', sharey='row')
#fig.tight_layout()
norm = colors.Normalize(vmin=-8, vmax=8)
images = []
for i in range(numBrains):
    weights, biases = read_model(fileobj)
    images.append(ax[i % VERT_GRAPHS][int(i // VERT_GRAPHS)].imshow(weights[0], cmap="hsv"))
    images[i].set_norm(norm)

fileobj.close()

fig.colorbar(images[0], ax=ax, orientation="horizontal", fraction=.1)
plt.show()
