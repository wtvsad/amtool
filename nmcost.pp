#include <iostream> 
#include <cmath>
#include <string>
#include <iomanip>
#include <locale>

using namespace std;

int main()
{
    cout.imbue(locale("en_US.UTF-8"));
    cout << fixed << setprecision(0);
    string currencyName;
    string petName;
    char choice;
    int petCost;
    double grandTotalBase = 0;

    cout << "\nHello, Adopt Me player!" << endl;
    cout << "Here to calculate how much you need to get your desired pet?" << endl;
    
    cout << "\nNo worries, start by entering the currency name!" << endl;
    cout << "Currency Name: ";
    getline(cin, currencyName);

    cout << "\nThe currency name is " << currencyName << "? Great!" << endl;

    do
    {
        cout << "\nGive me a name for your desired pet." << endl;
        cout << "Pet Name: ";
        getline(cin, petName);

        cout << "\nHow much does one " << petName << " cost in " << currencyName << "?" << endl;
        cout << "Cost of one " << petName << ": ";
        cin >> petCost;
        
        cout << "\nCalculation for only " << petName << endl;
        cout << "----------------------------------------" << endl;
        cout << "Cost of one " << petName << " is: " << petCost << " " << currencyName << endl;
        cout << "To make a NEON you need: " << petCost*4 << " " << currencyName << endl;
        cout << "To make a MEGA you need: " << petCost*16 << " " << currencyName << endl;
        cout << "For both NEON and MEGA, you need in total: " << petCost*20 << " "<< currencyName << endl;
        cout << "For NEON, MEGA and a base pet, you need in total: " << petCost*21 << " " << currencyName << endl;

        grandTotalBase += petCost;

        cout << "\nDo you want to add another pet to the calculation? If not, we will start total calculation.(Y/N): ";
        cin >> choice;
        cin.ignore();
    }
    while (choice == 'Y' || choice == 'y');


    cout << "\nCalculation for all pets entered" << endl;
    cout << "----------------------------------------" << endl;
    cout << "To get one of each pet, you need: " << grandTotalBase << " " << currencyName << endl;
    cout << "To make NEONs, you need: " << grandTotalBase*4 << " " << currencyName << endl;
    cout << "To make MEGAs, you need: " << grandTotalBase*16 << " " << currencyName << endl;
    cout << "For both NEONs and MEGAs, you need in total: " << grandTotalBase*20 << " "<< currencyName << endl;
    cout << "For NEONs, MEGAs and base pets, you need in total: " << grandTotalBase*21 << " " << currencyName << endl;
    cout << "Have fun grinding!" << endl;

    return 0;

}
