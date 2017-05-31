# Poročilo 2 #
- Dodano je bilo še nekaj logike za samo igro
- Popravljeno glede classov in headerjev. Imel sem nekaj težav z inicializacijo statičnih arrayev (first time c++)
- Končno narejeno že nekaj kar se tiče dejanskega vida:
    - Najprej najde kvadrat, ki predstavlja igralno plošlo, in se na to fokusira le na to
        - Popraviti se mora še dinamični resize in rotacija, trenutno je hardkodiran
    - Najde kroge na plošči
    - Naredi maske glede na barve. To bo morda tudi potrebno še tweakat, ker je mogoče preveč fiksirano na ta primer
    - Pobarva najdene kroge z ustrezno barvo.

Kmalu še:
- Krogi pripadajo ustrezni vlogi (štalca, izven igre, normalni krogi)
- Odziv AI

Težave:
- Prepoznavanje figure iste barve na isti plošči
