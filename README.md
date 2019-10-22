# ERA GP

## Tools
[Trello](https://trello.com/b/ol7c7Udk/evolution)

## Meetings
Wednesday, 12:00 - 13:00 in main hall of the MI building.

## Coding guidelines
Link to Trello task as first line in commit message.

## Commandline arguments

*Important*: The value must be written immediately after the argument (i.e. `-w720` and not `-w 720`)!

- **Width** `-w` (optional): Integer value declaring the width (in pixels) of the whole world. Default is 960px.

- **Height** `-h` (optional): Integer value declaring the height (in pixels) of the whole world. Default is 720px.

- **Food Spawn Rate** `-f` (optional): Float value that defines the amount of food spawned per 2000 tiles per tick. Default is 1.

- **MaiMUC** `-m` (optional): Run the application with the MaiMUC configuration. No value required.

## Using the keyboard and mouse while running the application

- **Pause/Play** `P`: Pauses simulation for further inspection of entities. Hitting the key again resumes simulation.

- **Similarity mode** `S`: Pauses simulation and two living entities can be selected via left-clicks. After that the
similarity of both living entities will be printed into the console. Hitting the key again resumes simulation and
disables the similarity mode.

- **Draw borders** `B`: Draws the borders of the world onto the window. This can help to easier tell apart two worlds
next to each other.

- **Quit** `Q`: Stops simulation and quits the application.

- **Left-click**: Get information about the nearest living entity, where the mouse was pressed, printed in the console.

## Run on MaiMUC

*Note*: execute every command in the project's root folder. **Don't** execute ``deploy.sh`` and ``run.sh`` in the local
dev environment!

After building the application, the executable and resources must be transferred to all other nodes on the MaiMUC. This
can be done with the following command:
```
./utils/deploy.sh
```

To start running the application on MaiMUC (with MPI) use the following command/script:
```
./utils/run.sh
```

## Local Dev Env
A docker container exists for easy local development.
The docker container exposes a VNC server at the address `vnc://localhost:5901` with password ``vncpassword``.
It also exposes a HTML client directly accessible in a browser at ``http://localhost:6901/?password=vncpassword``

All commands assume they are executed inside this project folder.

### Start the Container

```
docker run --rm -p 5901:5901 -p 6901:6901 -v $(pwd):/app --name eragp tobiashanl/eragp-evolution 
```

For Windows Powershell:
```
docker run --rm -p 5901:5901 -p 6901:6901 -v ${PWD}:/app --name eragp tobiashanl/eragp-evolution 
```
For my notebook: (sorry for misusing this)
```
docker run --rm -p 5901:5901 -p 6901:6901 -v "//c/users/jonas/OneDrive - tum.de/Dokumente/Studium/02 - SS 2019/ERA GP/CLion/eragp-maimuc-evo-2019:/app" --name eragp tobiashanl/eragp-evolution
```

### CMake (init & build) and execute the application

Initially and after changes to cmake, cmake has to be loaded:
```
docker exec -it eragp sh -c './utils/init.sh'
```
Build the project and execute it:
```
docker exec -it --user 0 eragp sh -c './utils/build.sh && mpirun ./build/Evolution'
```
(Parameters can be specified after ``mpirun``, i.e. ``mpirun -np 4``, and ``./build/Evolution``. See *Commandline
arguments* for more information)

Without .sh files (execute in ``build`` ):
```
cmake --build . --target Evolution && mpirun -n 6 ./Evolution
```

### Build the Container

Optionally, you can also build the container yourself
```
docker build --tag=eragp .
```
and then use `eragp` instead of `tobiashanl/eragp-evolution` in the run command.
