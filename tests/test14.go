package main

func main () {
	a := "str"
	a, _ = substr("abcdef", 1, 2)
	print("Toto je substr pro abcdef,1,2: ", a, "\n")
	b := 1
	b, _ = ord("ABCDEF", 2)
	print("Toto je pro ABCDEF,2: ", b, "\n")
	c := "str"
	c, _ = chr(66)
	print("hodnota c: ", c, "\n")
}
