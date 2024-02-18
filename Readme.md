# FastKV
Project created to learn seastar library. It's simple key value storage implementation which uses multiple threads to read/write key+value to the storage and cache.

## Build

### Build docker for running fastkv
```docker build -f Dockerfile-run -t fastkv-run .```

### Run fastkv
```docker run -p 8899:8899 --rm -it fastkv-run```
it will start fastkv application on 8899 port

You can use postman to trigger endpoints:
- POST request to store key/value:
`http://localhost:8899/store` with content:
`{key};{value}` for example 2455ff%5555;VAL45

- GET request to get value:
`http://localhost:8899/get/{key}` for example: `http://localhost:8899/get/2455ff%5555`

- GET request to remove key and value:
`http://localhost:8899/delete/{key}` for example: `http://localhost:8899/delete/2455ff%5555`

- GET request to get all keys:
`http://localhost:8899/getkeys/`

### Run e2e tests
While running fastkv-run container run:
```
cd tests/e2e
docker build -t fastkv-tests .
docker run --network=host fastkv-tests
```

or
```
cd tests/e2e
go test -v
```

## Development

### Build docker for development purposes
```
docker build -t fastkv .
docker run -p 8899:8899 --rm -it -v $(pwd):/home/src fastkv
```

## Run built app
```
./fastkv --cache-size=100
```
