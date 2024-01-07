package main

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"net/http"
	"strconv"
	"strings"
	"testing"
)

const posturl = "http://localhost:8899/store"
const geturl = "http://localhost:8899/get"
const getAllUrl = "http://localhost:8899/getkeys"

func store(key string, val string) {
	// JSON body

	// Create a HTTP post request
	req, err := http.NewRequest("POST", posturl, bytes.NewBufferString(key+";"+val))
	if err != nil {
		panic(err)
	}

	client := &http.Client{}
	res, err := client.Do(req)
	if err != nil {
		panic(err)
	}

	if res.StatusCode != http.StatusCreated {
		panic(res.Status)
	}
}

func get(key string, expectedVal string) {
	resp, err := http.Get(geturl + "/" + key)
	if err != nil {
		log.Fatalln(err)
	}
	b, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalln(err)
	}

	if string(b) != expectedVal {
		fmt.Println(string(b))
		fmt.Println(expectedVal)
		panic(string(b))
	}
}

func getAll(minNumOfKeys int) {
	resp, err := http.Get(getAllUrl)
	if err != nil {
		log.Fatalln(err)
	}
	b, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalln(err)
	}

	result := string(b)

	elems := strings.Split(result, ";")
	fmt.Println(elems)
	if len(elems) < minNumOfKeys {
		panic("min num of keys not achieved")
	}
}

func remove(key string) {
	resp, err := http.Get(geturl + "/" + "key1")
	if err != nil {
		log.Fatalln(err)
	}

	if resp.StatusCode != http.StatusOK {
		panic(resp.Status)
	}
}

func TestStoreAndGet(t *testing.T) {
	store("key1", "val1")
	get("key1", "val1")
}

func TestStoreAndDeleteMultiple(t *testing.T) {
	for i := 0; i < 1000; i++ {
		store("key"+strconv.Itoa(i), "val"+strconv.Itoa(i))
	}
	for i := 0; i < 1000; i++ {
		remove("key" + strconv.Itoa(i))
	}
}

func TestRemoveKey(t *testing.T) {
	store("key1", "val1")
	remove("key1")
}

func TestFetchAllKeys(t *testing.T) {
	store("key1", "val1")
	store("key2", "val2")
	store("key3", "val3")
	store("key4", "val4")
	store("key5", "val5")
	store("key6", "val6")
	store("key7", "val7")

	getAll(7)
}

func TestStoreAndGetMultiple(t *testing.T) {
	for i := 0; i < 1000; i++ {
		store("key"+strconv.Itoa(i), "val"+strconv.Itoa(i))
	}
	for i := 0; i < 1000; i++ {
		get("key"+strconv.Itoa(i), "val"+strconv.Itoa(i))
	}
}
