# IFJ-Compiler
IFJ Compiler source code
------------------------
    Nakonec jsme to docela zandali

Co budeme potřebovat pro pohodlnou práci na gitu:
-------------------------------------------------
    1) Nastavit SSH
    2) Používat Projects tab, kde je To-Do apod.
    3) Naučit se správně používat git
  
SSH
---

Pokud si nastavíte SSH, tak při uploadování vaší verze kódu nebudete muset zadávat nickname a heslo, což po asi desáté začne být maximálně otravné. Nejdřív musíte ve svém terminálu/git bashy vygenerovat SSH klíč (pozn. vytvořte si složku $HOME/.ssh a vstupte do ní). Ale pro windows je to trochu složitější: https://medium.com/rkttu/set-up-ssh-key-and-git-integration-in-windows-10-native-way-c9b94952dd2c v terminálu zadáte:

    ssh-keygen -t rsa -b 4096 -C "Gitemail@gitemail.cokoliv"

generátor se vás zeptá na pár věcí, doporučuji nechat jako název souboru id_rsa, passphase je vlastně heslo pro ssh klíč, rozhodně doporučuji, nejlépe silné heslo.
tento klíč by se vám měl uložit do .ssh složky, v něm najdete id_rsa.pub. Tento klíč nastavíte do Github>settings>SSH and GPG keys>new SSH key
Po uložení napište do terminálu/git bashe:
    
    ssh -T git@github.com
    
Bash se vás zeptá, zda má uložit/věřit fingerprintu, napište "yes"
Github by vám měl napsat, že jste se úspěšně ověřili.
Poté uložte repozitář IFJ-Compiler pomocí ssh:
    
    git clone git@github.com:DanielKriz/IFJ-Compiler.git
    
Nyní už nemusíte otravně zadávat heslo.

Projects tab
------------

v liště vidíte záložku projects, je tam dedikovaný to-do, in progress, done.
Pokud na něčem pracujte, prosím vepište příslušné infromace a přesuňte do in-progress.
Na rozdělení se případně domluvme.

Jak používat git
----------------

Jelikož si nechceme přepisovat kód co napíšeme, tak musíme k mergeování apod. přistupovat trochu opatrnějí.
Nejlepší a hlavně nejbezpečnější přístup je vytvořit si vlastní větev programu a tu následně opatrně a promyšleně mergenout s master branch.
Jak na to:

    git branch moje_vetev
    git checkout moje_vetev // nastaví, že děláte změny ve vaší větvi
    git push origin moje_vetev // origin je implicitní, jedná se o gitový server
    
Než však můžete něco pushnout, tak si musíte nastavit "globální identitu" a taky musíte ten upravený kód přidat musíte přidat do "nákupního vozéku", lidově řečeno.
Nastavení identiti:

    git config --global user.email "Váš@Email.cokoliv"
    git config --global user.name "Jméno Příjmení"
    
Přidání:

    git add soubor.c // připadně git add .  pro přidání všeho, ale to prosím dělejte jenom, pokud jste například v několika souborech upravovali vzhled
    git commit -m "zpráva" // zpráva by měla říkat co jste udělali, př. "added TYPE_INT tokenizer grammar"
    
po tomto můžete úspěšně pushnout

Dobrý zdroj informací:
https://blog.hipolabs.com/how-to-work-in-a-team-version-control-and-git-923dfec2ac3b

Štábní kultura
--------------
Je to hodnocená a hlavně i rozumné. Budeme tady deklarovat pravidla, kterými se budeme snažit řídit.
Obecně se uvádí limit na délku řádku 80 znaků, používá se to ve většina praxe. Další možností je 100 znaků na řádek, to ale není zcela standartní. Dejte mi vědět co preferujete.
Jaký bude tvar if-else apod.? Já (Dan) jsem si navykl na umístění { hned po závorce, ale hodně lidí to dává na nový řádek. př.

    if (something) {
        do_this();
    } else {
        do_that();
    }

Obdobně pro switch, for, while, do-while apod. Další možnost:

    if (something)
    {
        do_this();
    }
    else
    {
        do_that();
    }

Na tomdle se musíme domluvit.
Kvůli dokumentaci bych zavedl, že před každou funkci budeme psát něco podobného jako doxygen docs string. Stylem:

    /**
     * func(): Does that
     * int x: represends this
     * int y: represends that
     * return: returns this
     */
     func(int x, int y)

Typickým zvykem je konstanty (enum, #define i const) psát velkými písmeny,
proměnné by měli říkat co jsou, funkce by měli říkat co dělají.
Do header souborů uvádějte tzv. header guard. tj.
    
    #ifndef NAME_OF_HEADER_H
    #define NAME_OF_HEADER_H
    /* some code */
    #endif // NAME_OF_HEADER_H

Pomáhá to při překladu, aby se header nenalinkoval vícekrát => nebyl error.
