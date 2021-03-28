// Program 1: Vypocet faktorialu (iterativne)
package main

func main() {
  print("Zadejte cislo pro vypocet faktorialu: ")
  fero := 0
  fero = 2 + 2 + 4 + 5
	// fero, _ = inputi()
  if fero < 0 {
    print("Faktorial nejde spocitat!\n")
  } else {
    jozo := 1
    for ; fero > 0; fero = fero - 1 {
      jozo = jozo * fero
    }
    print("Vysledek je ", jozo, "\n")
  }
}
