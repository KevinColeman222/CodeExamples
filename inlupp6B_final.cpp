#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>
using namespace std;

//Globala konstanter
int const MAX_PERSONER = 6;
int const MAX_TRANSAKTIONER = 100;

//Klassdefinitioner

class Transaktion //Lagrar värden som hör ihop med ett kvitto
{
private:
  string datum;
  string typ;
  string namn;
  double belopp;
  int    antal_kompisar;
  string *kompisar; //Ny dynamisk array

public:
    Transaktion();
    Transaktion & operator = (const Transaktion &t); //Överlagrad Tilldelningsoperator
    ~Transaktion(); //Destruktor
    string hamtaNamn() const; //Namn på person som betalat
    double hamtaBelopp() const;
    int    hamtaAntalKompisar() const;
    bool   finnsKompis(const string &namnet) const;
    bool   lasIn(istream &is);
    void   skrivUt(ostream &os) const;
    void   skrivTitel(ostream &os) const;
  
};

class Person
{
private:
    string namn;
    double betalat_andras; //ligger ute med totalt
    double skyldig; //skyldig totalt
public:
    Person();
    Person(const string &namn, double betalat_andras, double skyldig);
    string hamtaNamn();
    double hamtaBetalat();
    double hamtaSkyldig();
    void skrivUt(ostream &os);
    bool ta_fran_pott(); //Lägger till denna för att kunna använda i personlista
                                  //Returnerar true om person ska ta från pott och false om person ska ge till pott
};

class PersonLista
{
private:
    int antal_personer;
    Person *personer; //Ny dynamisk array
    
public:
    PersonLista();
    ~PersonLista();
    void laggTill(Person ny_person);
    void skrivUtOchFixa(ostream &os);
    double summaSkyldig() const;
    double summaBetalat() const;
    bool finnsPerson(const string& namn);
};


class TransaktionsLista
{
private:
    int antal_transaktioner;
    Transaktion *transaktioner; //Ny dynamisk array
    
public:
    TransaktionsLista();
    ~TransaktionsLista();
    
    void lasIn(istream & is);
    void skrivUt(ostream &os);
    void laggTill(Transaktion & t);
    double totalKostnad();
    double totalKostnad() const;
    double liggerUteMed(const string &namnet);
    double arSkyldig(const string &namnet);
    PersonLista FixaPersoner();
};

//Huvudkod
int main()
{
    cout << "Startar med att läsa från en fil." << endl;

     TransaktionsLista transaktioner;
     std::ifstream is("resa.txt");
     transaktioner.lasIn(is);

     int operation = 1;
     while (operation != 0)
       {
         cout << endl;
         cout << "Välj i menyn nedan:" << endl;
         cout << "0. Avsluta. Alla transaktioner sparas på fil." << endl;
         cout << "1. Skriv ut information om alla transaktioner." << endl;
         cout << "2. Läs in en transaktion från tangentbordet." << endl;
         cout << "3. Beräkna totala kostnaden." << endl;
         cout << "4. Hur mycket är en viss person skyldig?" << endl;
         cout << "5. Hur mycket ligger en viss person ute med?" << endl;
         cout << "6. Lista alla personer mm och FIXA" << endl;

         cin >> operation;
         cout << endl;

         switch (operation)
           {
             case 1:
               {
                 transaktioner.skrivUt(cout);
                 break;
               }
             case 2:
               {
                 Transaktion transaktion;
                 cout << "Ange transaktion i följande format" << endl;
                 transaktion.skrivTitel(cout);
                 transaktion.lasIn(cin);
                 transaktioner.laggTill(transaktion);
                 break;
               }
             case 3:
               {
                 cout << "Den totala kostnanden för resan var "
                      << transaktioner.totalKostnad() << endl;
                 break;
               }
             case 4:
               {
                 cout << "Ange personen: ";
                 string namn;
                 cin >> namn;
                 double ar_skyldig = transaktioner.arSkyldig(namn);
                 if (ar_skyldig == 0.0)
                   cout << "Kan inte hitta personen " << namn << endl;
                 else
                   cout << namn << " är skyldig " << ar_skyldig << endl;
                 break;
               }
             case 5:
               {
                 cout << "Ange personen: ";
                 string namn;
                 cin >> namn;
                 double ligger_ute_med = transaktioner.liggerUteMed(namn);
                 if (ligger_ute_med == 0.)
                   cout << "Kan inte hitta personen " << namn << endl;
                 else
                   cout << namn << " ligger ute med " << ligger_ute_med << endl;
                 break;
               }
             case 6:
               {
                 cout << "Nu skapar vi en personarray och reder ut det hela!" << endl;
                 PersonLista lista = transaktioner.FixaPersoner();
                 lista.skrivUtOchFixa(cout);
                 break;
               }
           }
       }

     std::ofstream os("transaktioner.txt");
     transaktioner.skrivUt(os);

     return 0;
}

// -------METODER---------

//----Metoder till klassen "Transaktion"----

//Konstruktorer
Transaktion::Transaktion() //Standardkonstruktor
{
    kompisar=0; //NUll-pekare
}

Transaktion& Transaktion::operator=( const Transaktion& t)
{
    if (this != &t)
    {
        delete[] kompisar;
        datum          = t.datum;
        typ            = t.typ;
        namn           = t.namn;
        belopp         = t.belopp;
        antal_kompisar = t.antal_kompisar;
        kompisar       = new string[antal_kompisar];
        for (int i=0; i < antal_kompisar; i++)
        kompisar[i] = t.kompisar[i];
    }
    return *this;
}

//Destruktor
Transaktion::~Transaktion()
{
    delete [] kompisar; //Delete på pekaren kompisar
}

//Selektorer
string Transaktion::hamtaNamn() const
{
    return namn;
}

double Transaktion::hamtaBelopp() const
{
    return belopp;
}

int Transaktion::hamtaAntalKompisar() const
{
    return antal_kompisar;
}

//Övriga metoder
bool Transaktion::finnsKompis(const string &namnet) const
{
    //bool finns_kompis;
    for(int i=0; i<antal_kompisar; i++)
    {
        if (kompisar[i] == namnet)
        {
            return true;
        }
    }
    return false;
}

bool Transaktion::lasIn(istream & is)
{
    if(is) //Om filen ens finns
    {
        if (!is.eof()) //Om filen är slut
        {
            is >> datum >> typ >> namn >> belopp >> antal_kompisar;
            
            kompisar = new string[antal_kompisar]; //Allokerar minne
            for(int i=0; i<antal_kompisar; i++)
                is >> kompisar[i];
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

void Transaktion::skrivUt(ostream &os) const
{
    os << setiosflags(ios::left) << setw(8) << datum << setw(8) << typ << setw(8) << namn << setw(8) << belopp << setw(8) << antal_kompisar << setw(8); //Skriver ut information om transaktionserna
    for (int i=0; i<antal_kompisar; i++)
    {
        os << kompisar[i] << setw(8);
    }
    os << endl;
}

void Transaktion::skrivTitel(ostream &os) const
{
    os << setiosflags(ios::left) << setw(8) << "Datum" << setw(8) << "Typ" << setw(8) << "Namn" << setw(8) << "Belopp" << setw(8) << "Antal och lista av kompisar" << endl;
}

//----Metoder till klassen "TransaktionsLista"----

//Konstruktorer

TransaktionsLista::TransaktionsLista()
{}

//Destruktor
TransaktionsLista::~TransaktionsLista()
{
    delete [] transaktioner;
}

//Övriga metoder

void TransaktionsLista::lasIn(istream & is)
{
    if(is) //Om det ens finns en fil
    {

        int i=0;
        transaktioner = new Transaktion[i+1]; //Allokerar minne
        transaktioner[i].lasIn(is); //Läser in första
        
        while(is) //Medan filen inte har nått sitt slut så fortsätter metoden läsa in
        {
            i ++;
            //Nu förlänger vi transaktioner
            Transaktion *p =0; //Lokal variabel
            p = new Transaktion[i+1]; //Allokerar minne
            for (int j=0; j<i; j++) //Kopierar innehållet i transaktioner fram till förra antal_transaktioner
            {
                p[j] = transaktioner[j];
            }
            p[i].lasIn(is) ; //Läser in en till transaktion på sista platsen
            
            delete [] transaktioner; //Avallokerar minnet i transaktioner
            transaktioner = p; //Sätter transaktioner till den lokala arrayen
            
        }
        antal_transaktioner = i;
    }
    else
    {
        cout << "Det finns ingen fil!" << endl;
    }
}

void TransaktionsLista::skrivUt(ostream & os)
{
    cout << "Antal trans = " << antal_transaktioner << endl;
    transaktioner[0].skrivTitel(os); //Kallar på metoden i "Transaktion" som skriver ut översta titelraden
    
    for(int i=0; i<antal_transaktioner; i++)
    {
        transaktioner[i].skrivUt(os);
    }
}

void TransaktionsLista::laggTill(Transaktion & t)
{
    //Om maximalt antal transaktioner inte överskrids läggs det till en transaktion på nästa plats
    // och antalet transaktioner på resan ökas med 1
    if(antal_transaktioner<MAX_TRANSAKTIONER)
    {
        antal_transaktioner ++;
        
        //Nu förlänger vi transaktioner
        Transaktion *s =0; //Lokal variabel
        s = new Transaktion[antal_transaktioner]; //Allokerar minne
        for (int i=0; i<antal_transaktioner-1; i++) //Kopierar innehåller i transaktioner fram till förra antal_transaktioner
        {
            s[i] = transaktioner[i];
        }
        s[antal_transaktioner-1] = t; ; //Läser in en till transaktion på sista platsen
        
        delete [] transaktioner; //Avallokerar minnet i transaktioner
        transaktioner = s; //Sätter transaktioner till den lokala arrayen
    }
    else
    {
        cout << "Maximala antalet transaktioner på denna resa är överskriden! " << endl;
    }
}

double TransaktionsLista::totalKostnad()
{
    double sum = 0.0;
    
    for(int i=0; i<antal_transaktioner; i++)
    {
        sum += transaktioner[i].hamtaBelopp();
    }
    return sum;
}

double TransaktionsLista::liggerUteMed(const string &namnet)
{
    double summa = 0.0;
    for (int i = 0; i < antal_transaktioner; i++)
    {
        if (transaktioner[i].hamtaNamn() == namnet)
        {
            //Ta fram andel av vad personen betalat av varje transaktion
            summa += transaktioner[i].hamtaBelopp() * (1.0 - 1.0 / (transaktioner[i].hamtaAntalKompisar() + 1));
        }
    }
    return summa;
}

double TransaktionsLista::arSkyldig(const string &namnet)
{
    double skyldig_summa = 0.0;
    for (int i=0; i<antal_transaktioner; i++)
    {
        if(transaktioner[i].finnsKompis(namnet))
        {
            //Ta fram personens andel av varje transaktion som denna ska betala in
            skyldig_summa += transaktioner[i].hamtaBelopp()/(transaktioner[i].hamtaAntalKompisar()+1);
        }
    }
    return skyldig_summa;
}

PersonLista TransaktionsLista::FixaPersoner()
{
    PersonLista personer; //Lägger till objektet personer som ska vara alla personer i transaktionslistan
    
    for(int i=0; i<antal_transaktioner; i++)
    {
        string namn_betalande = transaktioner[i].hamtaNamn(); //Namn på personen som betalade
        double betalat_betalande = liggerUteMed(namn_betalande);
        double skyldig_betalande = arSkyldig(namn_betalande);
        Person person_temp(namn_betalande, betalat_betalande, skyldig_betalande); //Skapar ett temporärt objekt person_temp
        
        if(!personer.finnsPerson(namn_betalande)) //Om personen är ej tillagd så läggs den till personlistan
        {
            personer.laggTill(person_temp);
        }
    }

    return personer;
}


//----Metoder till klassen "Person"----

//Konstruktorer

Person::Person()
{}

Person::Person(const string &namn, double betalat_andras, double skyldig)
    :namn(namn), betalat_andras(betalat_andras), skyldig(skyldig)
{}

//Selektorer

string Person::hamtaNamn()
{
    return namn;
}

double Person::hamtaBetalat()
{
    return betalat_andras;
}

double Person::hamtaSkyldig()
{
    return skyldig;
}

//Övriga metoder

void Person::skrivUt(ostream &os)
{
    if(skyldig<betalat_andras) //Om personen ska ha från potten
    {
        os << namn << " ligger ute med " << betalat_andras << " och är skyldig " << skyldig << ". Skall ha " << betalat_andras-skyldig << " från potten! " << endl;
    }
    else if (skyldig>betalat_andras) //Om personen ska ge till potten
    {
        os << namn << " ligger ute med " << betalat_andras << " och är skyldig " << skyldig << ". Skall lägga " << skyldig-betalat_andras << " till potten! " << endl;
    }
    else //Om summorna är exakt lika stora
    {
        os << namn << " ligger ute med " << betalat_andras << " och är skyldig " << skyldig << ". Ska inte ha från potten eller ge något till potten. Personen är kvitt alla!" << endl;
    }
}

bool Person::ta_fran_pott()
{
    if (skyldig<betalat_andras)//Detta innebär att person ska ta från potten och därmed returneras true
    {
        return true;
    }
    else if (skyldig>betalat_andras) //Innebär att person ska ge till potten.
    {
        return false;
    }
    else //Om de är lika med varandra så kommer ändå det som ska tillkomma eller tas ifrån potten vara 0, vilket inte
        //påverkar resultatet
    {
        return false;
    }
}

//----Metoder till klassen "PersonLista"----

//Konstruktor
PersonLista::PersonLista()
{}

//Destruktor
PersonLista::~PersonLista()
{
    delete [] personer;
}

//Andra metoder
void PersonLista::laggTill(Person ny_person)
{
    //Om maximalt antal personer inte överskrids läggs det till en person på nästa plats
    // och antalet personer på resan ökas med 1

    if(antal_personer<MAX_PERSONER)
    {
        if(antal_personer==0) //Om det är första gången vi lägger till en person
        {
            antal_personer++;
            Person *p =0; //Lokal variabel
            p = new Person[antal_personer];
            p[antal_personer-1] = ny_person;
            
            delete [] personer; //Avallokerar minnet i personer
            
            personer = p; //Sätter personer till den lokala arrayen
        }
        else
        {
            antal_personer++; //Annars lägger vi till en person
            
            //Nu förlänger vi personer
            Person *p =0; //Lokal variabel
            p = new Person[antal_personer];
            for (int i=0; i<antal_personer-1; i++) //Kopierar innehåller i transaktioner fram till förra antal_transaktioner
            {
                p[i] = personer[i];
            }

            p[antal_personer-1] = ny_person;//Läser in en till person på sista platsen
            
            delete [] personer; //Avallokerar minnet i personer
            
            personer = p; //Sätter personer till den lokala arrayen
        }
        
    }
    else
    {
        cout << "Maximala antalet personer på resan är överstigen! " << endl;
    }
}

void PersonLista::skrivUtOchFixa(ostream &os)
{
    double summa_skyldig = summaSkyldig();
    double summa_betalat = summaBetalat();
    double diff = summa_skyldig-summa_betalat;
    
    if(diff<0.001) //Om det som går in och ut ur potten stämmer så skriver vi ut resultatet
    {
        for(int i=0; i<antal_personer; i++) //Går igenom array med personerna med på resan och använder metoden "skrivUt" i
        {                                   //"Person" klassen för att skriva ut info om allas slutgiltiga status
            personer[i].skrivUt(cout);
            cout << endl;
        }
    }
    else //Om summorna inte stämmer är det något fel med bokföringen eller programmet
    {
        cout << "Summorna stämmer inte! Något har blivit fel vid bokföring av kvitton eller på programmet..." << endl;
    }
}

double PersonLista::summaSkyldig() const
{
    double ge_till_pott=0.0; //Summa av det som tillförs till potten
    //Om allt stämmer ska denna vara lika med utskrift från summaBetalat.
    
    for(int i=0; i<antal_personer; i++) //Går igenom array med personerna med på resan och beräknar totala summan av pengar som ska till potten
    {
        if (!personer[i].ta_fran_pott()) //Person ska ge till pott om detta stämmer
        {
            ge_till_pott += (personer[i].hamtaSkyldig() - personer[i].hamtaBetalat());
            //Lägger till det som ska tas från potten för denna person till total summa
        }
    }
    return ge_till_pott;
}

double PersonLista::summaBetalat() const
{
    double ta_fran_pott=0.0; //Summa av det som ska tas ut potten
    //Om allt stämmer ska denna vara lika med utskrift från summaSkyldig.
    
    for(int i=0; i<antal_personer; i++) //Går igenom array med personerna med på resan och beräknar totala summan av pengar som ska till potten
    {
        if (personer[i].ta_fran_pott()) //Person ska ta från pott om detta stämmer
        {
            ta_fran_pott += (personer[i].hamtaBetalat() - personer[i].hamtaSkyldig());
            //Lägger till det som ska tas från potten för denna person till total summa
        }
    }
    return ta_fran_pott;
}


bool PersonLista::finnsPerson(const string& namn)
{

    int j=0; //Index i unika_personer_array
    bool hittad = false;

    while(!hittad && (j<antal_personer)) //Går igenom personlistan och kollar om namnen i arrayen med person objekt
    {                                                           //innehåller namnet som skickas in

        if(personer[j].hamtaNamn()==namn)
        {
            hittad=true;
        }
        else
        {
            j++;
        }
    }
    
    if(hittad) //Om vi tittat igenom hela arrayen och hittade personen
    {
        return true;
    }
    else
    {
        return false;
    }
}



