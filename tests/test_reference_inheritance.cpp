#include <iostream>
#include <vector>

// #include "../game_files/Decks.h"
// #include "../game_files/Map.h"

using namespace std;

class A{
    int& my_attribute;
public:
    // Overloaded constructor
    A(int attr): my_attribute(attr){};
    A(int& attr): my_attribute(attr){};

    void set_attr(int i){
        my_attribute = i;
    };
    int& get_attr(){
        return my_attribute;
    };
    
};

class B{
    A& my_class; // attribute is a reference to type <A> class instantiation
public:
    B(A& example): my_class(example){};
    A& get_class(){return my_class;};

    void add_one(){
        my_class.set_attr(my_class.get_attr()+1);
    };
    int get_attr(){
        return my_class.get_attr();
    };
};

std::vector<A> GLOBAL_DEFINITION = {A(1),A(2),A(3)};

int main(){

    A& my_example = GLOBAL_DEFINITION[0]; // Reference to first element of GLOBAL_DEFINITION
    int& my_var = my_example.get_attr();  // Reference to integer value in my_example
    int& other_var = my_var;              // Reference to integer reference
    B container_class = B(my_example);    // class with reference attribute my_example

    cout << "Reference of type <A> to GLOBAL_DEFINITION[0]: " << my_example.get_attr() << endl;
    cout << "Reference type <int> to value contained in above class: " << my_var << endl;
    cout << "Reference type <int> to above variable: " << other_var << endl;
    cout << "Class with reference attribute to reference of type <A>: " << container_class.get_attr()<< endl;
    cout << "Original GLOBAL_DEFINITION[0] attribute value: " << GLOBAL_DEFINITION[0].get_attr() << endl;
    cout << "=====\n";

    cout << "Now I change the value in the GLOBAL_DEFINITION[0] from 1 to 10"<<endl;
    cout << "=====\n";

    GLOBAL_DEFINITION[0].set_attr(10);

    cout << "Reference of type <A> to GLOBAL_DEFINITION[0]: " << my_example.get_attr() << endl;
    cout << "Reference type <int> to value contained in above class: " << my_var << endl;
    cout << "Reference type <int> to above variable: " << other_var << endl;
    cout << "Class with reference attribute to reference of type <A>: " << container_class.get_attr()<< endl;
    cout << "Original GLOBAL_DEFINITION[0] attribute value: " << GLOBAL_DEFINITION[0].get_attr() << endl;
    cout << "=====\n";

    
    // int& my_var = one_example.get_attr();
    // cout << "int& my_var = one_example.get_attr();\n";
    // cout<< "my_var after assignment: " << my_var << endl;

    // cout << "Now change class attribute...(one_example.set_attr(5))\n";
    // one_example.set_attr(5);
    // cout << "Class attribute is now " << one_example.get_attr() << endl;

    // cout << "my_var after change: " << my_var << endl;

    // int& my_other_var = other_example.get_attr();
    // cout << "int& my_other_var = other_example.get_attr();\n";
    // cout << "my_other_var after assignment: " << my_other_var << endl;

    // cout << "Change the value in GLOBAL_DEFINITION\n;";
    // GLOBAL_DEFINITION[0] = 100;
    // cout << "Now the class value is " << other_example.get_attr() << endl;
    // cout << "Now my_other_var value is " << my_other_var << endl;

}