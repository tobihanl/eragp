# ERA GP

## Tools
[Trello](https://trello.com/b/ol7c7Udk/evolution)

## Meetings
Wednesday, 12:00 - 13:00 in main hall of the MI building.

## Coding guidelines
Link to Trello task as first line in commit message.


## Local Dev Env
A docker container exists for easy local development.
The docker container exposes a VNC server under the address `vnc://localhost:5901`.

All commands assume they are executed inside this project folder

Start the container:
```
docker run --rm -p 5901:5901 -p 6901:6901 -v $(pwd):/app --name eragp tobiashanl/eragp-evolution 
```

Initially and after changes to cmake, cmake has to be loaded:
```
docker exec -it eragp sh -c 'cmake -DCMAKE_C_COMPILER=/usr/bin/mpicc .'
```
Build the project and execute it.
```
docker exec -it --user 0 eragp sh -c 'cmake --build . --target Evolution -- -j && ./Evolution'
```

To build the container yourself, execute these commands
```
docker build --tag=eragp .
```
and then use `eragp` instead of `tobiashanl/eragp-evolution` in the run command.
