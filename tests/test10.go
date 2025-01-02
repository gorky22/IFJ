package main

func test_func(i int) (int, int) {
	i = i + i
	a := 2
	return i, a
}

func main() {
	a := 1
	b := 3
	c := b

	print(a, b, 4, "Hello", c)

	if a > 2 {
		c = b + 2
	} else {
		c = 1
	}

	for i := 0; i < 5; i = i + 1 {
		print(i, "\n");
		a = a + 1
	}
}
