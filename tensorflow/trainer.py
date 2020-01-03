import tensorflow as tf
from tensorflow import keras
import numpy as np
import matplotlib.pyplot as plt

"""
data = keras.datasets.fashion_mnist
(train_images, train_labels), (test_images, test_labels) = data.load_data()

train_images = train_images/255.0
test_images = test_images/255.0

model = keras.Sequential([
    keras.layers.Flatten(input_shape=(28,28)),
    keras.layers.Dense(128, activation="tanh"),
    keras.layers.Dense(10, activation="softmax")
    ])

model.compile(optimizer="adam", loss="sparse_categorical_crossentropy", metrics=["accuracy"])

model.fit(train_images, train_labels, epochs=5)

test_loss, test_acc = model.evaluate(test_images, test_labels)
print("Test Acc", test_acc)
"""

VIEW_RANGE = 160
TILE_SIZE = 8
INPUT_SIZE = 14
OUTPUT_SIZE = 4
FILENAME = "brains.dat"
NUM_BRAINS = 100
"""
--------just food:------
choose:
currentRotation: [-1,1]
water(Agility): -1|1
targetDist: [0, 0.8]
maxDist: [0, 1]
targetDir: [-1, 1]
---
nearestFoodDist: targetdist
nearestEnemyDist: 1
nearestMateDist: 1
energy: [0, 1]
waterAgility: water(Agility)
maxDist: maxDist
mateEnergy: 1
enemySize: 0
nearestFoodDir: targetDir
nearestEnemyDir: currentRotation
nearestMateDir: currentRotation
water: water(Agility)
center: [-1,1]
danger: -1
---
rotation: targetDir
speed: 
attack: -1
share: -1
"""

def food_only():
    currentRotation = np.random.uniform(-1,1) #only in [-1,1), but doesn't matter
    water = np.random.randint(0,2) * 2 - 1
    targetDist = np.random.uniform(0,0.8)
    maxDist = np.random.uniform(0, 1)
    targetDir = np.random.uniform(-1,1)
    return np.array([
        targetDist,
        1,
        1,
        np.random.uniform(0,1),
        water,
        maxDist,
        1,
        0,
        targetDir,
        currentRotation,
        currentRotation,
        water,
        np.random.uniform(-1,1),
        -1
        ]), np.array([
        targetDir,
        1 if TILE_SIZE * maxDist < (VIEW_RANGE * targetDist) / 0.8 else (VIEW_RANGE * targetDist) / (TILE_SIZE * maxDist * 0.8),
        -1,
        -1
        ])

def generate_dataset(size):
    ins = np.empty((size, INPUT_SIZE))
    labels = np.empty((size, OUTPUT_SIZE))
    for i in range(size):
        ins[i], labels[i] = food_only()
    return ins, labels

def print_model(model):
    for layer in model.layers:
        weights = layer.get_weights()[0]
        biases = layer.get_weights()[1]
        print("{", end="")
        for i in range(weights.shape[0]):
            for j in range(weights.shape[1]):
                print(str(weights[i][j]) + (", " if i != weights.shape[0] - 1 or j != weights.shape[1] - 1 else ""), end="")
            if(i != weights.shape[0] - 1): print()
        print("}")
        print()
        print("{", end="")
        for i in range(biases.shape[0]):
            print(str(biases[i]) + (", " if i != biases.shape[0] - 1 else ""), end="")
        print("}")
        print()

def write_model_to_file(fileobj, model):
    np.array([3], dtype=np.int32).tofile(fileobj) #numLayers
    for layer in model.layers:
        weights = layer.get_weights()[0]
        biases = layer.get_weights()[1]

        np.array([weights.shape[0], weights.shape[1]], dtype=np.int32).tofile(fileobj)
        weights.tofile(fileobj) #should be float32 by default?
        
        np.array([1, biases.shape[0]], dtype=np.int32).tofile(fileobj)
        biases.tofile(fileobj)

fileobj = open(FILENAME, mode="wb")
np.array([NUM_BRAINS, 4*(1 + 2 + INPUT_SIZE*8 + 2 + 8 + 2 + 8*OUTPUT_SIZE + 2 + OUTPUT_SIZE)], dtype=np.int32).tofile(fileobj)

for i in range(NUM_BRAINS):
    print("############################### Training Brain " + str(i) + "/" + str(NUM_BRAINS) + " ###############################")
    train_ins, train_labels = generate_dataset(100000)
    test_ins, test_labels = generate_dataset(1000)
    
    model = keras.Sequential([
        keras.layers.Dense(8, activation="tanh", input_shape=(INPUT_SIZE,)),
        keras.layers.Dense(OUTPUT_SIZE, activation="tanh")
    ])
    model.compile(optimizer="adam", loss="mse", metrics=["mae"])

    model.fit(train_ins, train_labels, epochs=5, batch_size=32, validation_data=(test_ins, test_labels))

    print_model(model)
    write_model_to_file(fileobj, model)

fileobj.close()

"""
test_in, test_label = food_only()
print(test_in)
print(test_label)
valid_input = test_in.reshape(1, 14)
print(valid_input)
print(model.predict(valid_input))
"""
