CC = g++
CFLAGS = -O3

SRC = main.cpp

PROG = s3-fifo

$(PROG) : $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

build: $(PROG)

DATASET = dataset.txt

$(DATASET):
	python3 gen_requests.py --output_file=$(DATASET)

run: build $(DATASET)
	time -p ./$(PROG) < $(DATASET)

clean:
	rm $(PROG)