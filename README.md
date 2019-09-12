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

- **MaiMUC** `-m` (optional): Run the application with the MaiMUC configuration. No value required.

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
docker exec -it eragp sh -c 'cd ./build/ && cmake ./..'
```
Build the project and execute it:
```
docker exec -it --user 0 eragp sh -c 'cd ./build/ && cmake --build . --target Evolution && mpirun ./Evolution'
```
(Parameters can be specified after mpirun, i.e. mpirun -np 4 ./Evolution, and ./Evolution. See *Commandline arguments* for more information)

### Build the Container

Optionally, you can also build the container yourself
```
docker build --tag=eragp .
```
and then use `eragp` instead of `tobiashanl/eragp-evolution` in the run command.
