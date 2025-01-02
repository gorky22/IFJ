package main

func test_func(i int, c int, d string) (int, int) {
	i = i + i
	return i, c
}

func main() {
	a := 0
	b := 0

	a,b = test_func(42, 69, "hatlapatla")
}
