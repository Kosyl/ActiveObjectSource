ActiveObjectSource
==================
Projekt ma na celu utworzenie biblioteki pozwalającej na zdalne wywoływanie erupcji wulkanów na wyspach szczęśliwych. Umożliwia programiście zaprowadzenie pokoju na świecie w oddzielnym wątku, a także podróż dookoła świata w postaci strumienia pakietów RTP.

==================
- Jeśli chcesz, możesz spróbować podzielić te pliki na c i h, ja się boję za to brać bo nie potrafię. Jeśli chcesz to też mogę być za, ale będziesz musiała powiedzieć, które aktualnie dzielisz albo często gitować, bo jak ja coś zmienię i zacznę mergować jeden plik hpp z dwoma h i c, to się o nawiasy potnę, a o średniki zabiję
- Zrobienie metody do robienia metod do robienia metod :)
znaczy w Example1.hpp, żeby nie trzeba było pisać za każdym razem tego samego prawie wszędzie dla każdej jednej metody w klasie Proxy;
chyba da się to zrobić przez jakiś rozbudowany szablon, który będzie miał m.in. typ zwracany, jaiegoś binda/function, pewnie guarda...
- ...właśnie - guard()! Trzeba rozbudować MethodRequest o jeszcze jedną (oprócz tej "właściwej") komendę guarda, a do niej będziemy bindować inne metody z jakiegoś konkretnego Servanta (tzn. FutureContentWritera). Dobrym pomysłem jest zrobienie tej kolejki o ograniczonej pojemności komunikatów z metodami get, put, isEmpty i isFull jak w tym pdfie, i sparowanie: wywołanie Get na proxy robi nowy MethodRequest który przechowuje komendę Get servanta, a jako guarda przechowuje info, że ma wywołać isEmpty servanta (jeśli kolejka będzie pusta, to Get się nie może wywołać)
- Sprawdzić, jak się będą zachowywały Future i reszta dla wywołań zwracających:
void
typy użytkownika
wskaźniki
- Pomyśleć nad tym problemem z jednym wskaźnikiem na Content w Servancie, jak to rozwiązać dla dwóch lub więcej wątków żeby było ok
pomyślałem o Thread Specific Storage, było coś o tym na wykładzie, wtedy ten wskaźnik mógłby być thread-specific i celować w odpowiedni content, warto sprawdzić, czy to możliwe
- Poza tym doxygen
- Scons - mam chyba u siebie wirtualną Fedorę, jak uda się okiełznać Sconsa na Windowsie to przejdę tam i spróbuję coś zdziałać
- Przerobienie funkcji testujących na boost::test; logi mogą zostać, tylko oprócz nich damy po każdej fazie sprawdzanie warunków typu "upewnij się że nie ma wyjątku w Future", "upewnij się że getValue zwraca zero", "upewnij się, że stan komendy to teraz Queued" itp. itd. ibjwcj.
- Jeśli zadziała ten operator T() co go napisaliśmy dziś, to można sprawdzić co się stanie jeśli przekażemy Future jako parametr metody (np. Future<double> do metody przymującej double)
- Ew. priorytety
