import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors

FILENAME = "brains.dat"
VERT_GRAPHS = 5
WEIGHT_RANGE = 1
BIASES = False
LAYER = 1
if(BIASES):
    VERT_GRAPHS = 5 * VERT_GRAPHS

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
min = 1000
max = -1000
matrices = []
for i in range(numBrains):
    w, b = read_model(fileobj)
    if(BIASES):
        matrices.append(b[LAYER])
        if(b[LAYER].min() < min): min = b[LAYER].min()
        if(b[LAYER].max() > max): max = b[LAYER].max()
    else:
        matrices.append(w[LAYER])
        if(w[LAYER].min() < min): min = w[LAYER].min()
        if(w[LAYER].max() > max): max = w[LAYER].max()
fig, ax = plt.subplots(VERT_GRAPHS, int(numBrains/VERT_GRAPHS), sharex='col', sharey='row')
#fig.tight_layout()
norm = colors.Normalize(vmin=min, vmax=max)
images = []
for i in range(numBrains):
    ax[i % VERT_GRAPHS][int(i // VERT_GRAPHS)].set_axis_off()
    images.append(ax[i % VERT_GRAPHS][int(i // VERT_GRAPHS)].imshow(matrices[i], cmap="hsv"))
    images[i].set_norm(norm)

fileobj.close()

fig.colorbar(images[0], ax=ax, orientation="horizontal", fraction=.1)
plt.show()
