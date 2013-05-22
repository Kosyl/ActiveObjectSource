ActiveObjectSource
==================
Projekt ma na celu utworzenie biblioteki pozwalającej na zdalne wywoływanie erupcji wulkanów na wyspach szczęśliwych. Umożliwia programiście zaprowadzenie pokoju na świecie w oddzielnym wątku, a także podróż dookoła świata w postaci strumienia pakietów RTP.

==================
Możemy tego użyć jako jakiegos "todo list" moze? Tylko nie ma polfontow :(

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Udało Ci się, awesome. Jutro z samego rana zrobię lekkie dostosowanie moich .h do Twoich i wrzucam.

==================
class Funktor
{
  	public:
		virtual void execute()=0;
}

template<class T>
class Command: public Funktor
{
	protected:
		Promise<T> prom;
		b::function<T> command;
		
	public:
		void execute()
		{
			prom.setValue(command());
		}
		void setProgress(double p)
		{
			prom.setProgress(p);
		}
}

class Kalkulator: public Proxy
{
	private:
		class Dodawanie: public Command<int>
		{
			int wlasciwaFunkcja(int a, int b)
			{
				setProgress(0.3);
				int x=a+b;
				setPorgress(0.7);
				return x;
			}
			
			int Dodawanie(int a, int b)
			{
				command=b::bind(Dodawanie:wlasciwaFunkcja, this, a, b);
			}
		}
	public:
		Future<int> dodaj(int a, int b)
		{
			Dodawanie d(a,b)
			Future<int> res = d.getFuture();
			schedule(d);
			return res;
		}
};a
