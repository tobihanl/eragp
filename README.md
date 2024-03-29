# ERA GP

## Tools
[Trello](https://trello.com/b/ol7c7Udk/evolution)

## Coding guidelines
Link to Trello task in commit message.

## Commandline arguments (optional)

- **Width** `-w`: Integer value declaring the width (in pixels) of the whole world. Default is 1000px.

- **Height** `-h`: Integer value declaring the height (in pixels) of the whole world. Default is 1000px.

- **Food Spawn Rate** `-f`: Float value that defines the amount of food spawned per 2000 tiles per tick.
Default is 0.2.

- **Ticks** `-t`: Amount of ticks (`long`) to simulate. No limitation with value -1. Default is -1.

- **Render** `-r`: Render the application while running. No value required. Default is **no rendering**.

- **Zoom** `-z`: Zoom into the world. A (`float`) value greater/less than 1 zooms into/out of the world. Default is 3.
Only values greater than 0.1 are allowed.

- **MaiMUC** `-m`: Run the application with the MaiMUC configuration. No value required. Application **must
be** executed on 10 nodes with this configuration!

- **Random Seed** `-s`: Run application with given seed.

- **Amount of entities** `-e`: Specify the amount of entities to spawn at start-up on the **entire** world.
Entities will be distributed equally to all ranks. Format for argument: `{AMOUNT_LIVINGS},{AMOUNT_FOOD}`. Default is
`500,500`.

- **Frame-Rate (FPS)** `-p`: Set the maximum amount of frames-per-second (fps) to render. Must be an integer value.
Default is 10.

- **Log File** `-l`: Log data about application into a `.csv` file. Filename has to be specified within this
option. The following will be appended to the filename: `-{MPI_Rank}.csv`.

- **OpenMP Threads** `-o`: Run parallel sections with the specified number of threads. Default is 1 (only master 
thread).

- **Boarisch mode** `-b`: Enables the "Boarisch" mode (with pretzels)

- **Automated mode** `-a`: Disable the cli input for execution within scripts

## Resources
To render logos in the background, place them in the `res/logos` folder with names `<MPI-Rank>.png`.

## Using the keyboard and mouse (ONLY in render-mode)

- **Pause/Play** `P`: Pauses simulation for further inspection of entities. Hitting the key again resumes simulation.

- **Similarity mode** `S`: Pauses simulation and two living entities can be selected via left-clicks. After that the
similarity of both living entities will be printed into the console. Hitting the key again resumes simulation and
disables the similarity mode.

- **Draw borders** `B`: Draws the borders of the world onto the window. This can help to easier tell apart two worlds
next to each other. The borders are drawn in red.

- **Draw padding areas** `A`: Draws all padding areas from other nodes on the window of a node. The rectangles are drawn
in blue.

- **Quit** `Q`: Stops simulation and terminates the application.

- `Right-click`: Get information about the nearest living entity, where the mouse was pressed, printed in the console.

- `Left-click`: Spawn *beer* at the position pressed. Only in *Boarisch mode*, otherwise a normal Food-Entity will be
spawned.

## Using the CLI while running the application
You can type the same characters to the CLI as you would press in a window while in render-mode.
These are the only additions:

- **Hide** `H`: Hide the window and switch to *hidden-mode*. Can only be executed in *render-mode* and only if the
application was compiled with RENDER flag on (see Compiling).

- **Render** `R`: Switch to *render-mode* and show the window. Can only be executed in *hidden-mode* and only if the
application was compiled with RENDER flag on (see Compiling).

- **Quit** `Q`: Stops the simulation and terminates the application. This can also be called in *hidden-mode*.

## Compiling
All commands should be executed in the project root folder unless stated otherwise.

### Init Cmake
The following `<build type>`'s are available: `Debug`, `Release`, `RelWithDebInfo` and `MinSizeRel`.
For profiling, *RelWithDebInfo* should be used.

`<render>` flag can be `ON` or `OFF`.

```
./utils/init.sh <build type> <render>
```

### Build application
```
./utils/build.sh
```

## Running

### MaiMUC
> **WARNING**: perf on maimuc is really slow and takes very long even for as little as 5 ticks

See *compiling* above on how to init cmake and build the application.

Distribute the necessary files to all other nodes
```
./utils/deploy.sh
```
Run the application
```
./utils/run.sh ./Evolution -m -r
```
Profile the application and generate the flamegraphs
```
./utils/run.sh ./utils/profile-maimuc.sh <parameters>
```
Copy all svg files to the mai02 node
```
./utils/collect-svg.sh
```
Get the generated svg files (**run on your local machine**)
```
scp -oProxyCommand="ssh -W %h:%p <login>@himmuc.caps.in.tum.de" login@maimuc.caps.in.tum.de:~/evolution/eragp-maimuc-evo-2019/perf/*.svg .
```

### HimMuc
> **Warning: Ask for confirmation before using more than 20 nodes!**

[Official TUM Information](https://www.caps.in.tum.de/hw/himmuc/quick-start/) 

[SLURM with MPI Documentation](https://www.open-mpi.org/faq/?category=slurm#slurm-run-jobs)

See *compiling* above on how to init cmake and build the application. 
You need to use a node for that (the login vm does not work) and run `module load mpi` on it first.

Run the application
```
srun -N <number of nodes> ./Evolution <parameters>
```
Profile the application and generate the flamegraphs
```
srun -N <number of nodes> ./utils/profile-himmuc.sh <parameters>
```
Get the generated svg files (**run on your local machine**)
```
scp <login>@himmuc.caps.in.tum.de:~/eragp-maimuc-evo-2019/perf/*.svg .
```

### Ubuntu machine
Use profile-local.sh for example as follows:
```
sudo mpirun -n 4 ./utils/profile-local.sh -z4 -r -f0.1 -e100,50 -w1200 -h900 -t100
```
## Local Dev Env
A docker container exists for easy local development.
The docker container exposes a VNC server at the address `vnc://localhost:5901` with password ``vncpassword``.
It also exposes a HTML client directly accessible in a browser at ``http://localhost:6901/?password=vncpassword``

### Start the Container
MacOS
```
docker run --rm -p 5901:5901 -p 6901:6901 -v $(pwd):/app --name eragp tobiashanl/eragp-evolution 
```
Windows
```
docker run --rm -p 5901:5901 -p 6901:6901 -v ${PWD}:/app --name eragp tobiashanl/eragp-evolution 
```
Linux
```
sudo docker run --rm -p 5901:5901 -p 6901:6901 -v "$(pwd):/app" --name eragp tobiashanl/eragp-evolution 
```
### Run commands inside the container
See *compiling* above on how to init cmake and build the application. 

Get a bash
```
docker exec -it --user 0 eragp /bin/bash
```
Execute a command directly inside the project folder (/app)
```
docker exec -it --user 0 eragp /bin/bash -c '<command>'
```
Use mpirun to run the application
```
docker exec -it --user 0 eragp /bin/bash -c 'mpirun -n 4 ./Evolution <parameters>'
```
#### Alternative: Directly in the containers build folder
```
cmake --build . --target Evolution && mpirun -n 2 ./Evolution -z4 -r -f0.01 -e1000,100 -w1200 -h900
```
### Build the Container
Optionally, you can also build the container yourself
```
docker build --tag=eragp .
```
and then use `eragp` instead of `tobiashanl/eragp-evolution` in the run command.
