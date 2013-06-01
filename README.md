ActiveObjectSource
==================
Projekt ma na celu utworzenie biblioteki pozwalającej na zdalne wywoływanie erupcji wulkanów na wyspach szczęśliwych. Umożliwia programiście zaprowadzenie pokoju na świecie w oddzielnym wątku, a także podróż dookoła świata w postaci strumienia pakietów RTP.

==================
Ogladaj to prosze w trybie raw, widze ze zepsulo zupelnie formatowanie jak sie czyta readmie z glownej.

- Jeśli chcesz, możesz spróbować podzielić te pliki na c i h, ja się boję za to brać bo nie potrafię. Jeśli chcesz to też mogę być za, ale będziesz musiała powiedzieć, które aktualnie dzielisz albo często gitować, bo jak ja coś zmienię i zacznę mergować jeden plik hpp z dwoma h i c, to się o nawiasy potnę, a o średniki zabiję
- guard() - zrobione, potestować
- Sprawdzić, jak się będą zachowywały Future i reszta dla wywołań zwracających:
void - wydaje sie ok, potestowac
- Pomyśleć nad tym problemem z jednym wskaźnikiem na Content w Servancie, jak to rozwiązać dla dwóch lub więcej wątków żeby było ok
pomyślałem o Thread Specific Storage, było coś o tym na wykładzie, wtedy ten wskaźnik mógłby być thread-specific i celować w odpowiedni content, warto sprawdzić, czy to możliwe
UPDATE: raczej nie w tym przypadku, ale jeszcze do ogarniecia
- Poza tym doxygen
- Scons - mam chyba u siebie wirtualną Fedorę, jak uda się okiełznać Sconsa na Windowsie to przejdę tam i spróbuję coś zdziałać
- Przerobienie funkcji testujących na boost::test; logi mogą zostać, tylko oprócz nich damy po każdej fazie sprawdzanie warunków typu "upewnij się że nie ma wyjątku w Future", "upewnij się że getValue zwraca zero", "upewnij się, że stan komendy to teraz Queued" itp. itd. ibjwcj.
- Jeśli zadziała ten operator T() co go napisaliśmy dziś, to można sprawdzić co się stanie jeśli przekażemy Future jako parametr metody (np. Future<double> do metody przymującej double)
- Ew. priorytety
