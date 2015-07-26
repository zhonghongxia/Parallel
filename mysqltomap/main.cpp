#include "db_to_map.h"
int main()
{
	db_to_map db_map;
	db_map.init();
    string str = db_map.query("test0.jpg");
	cout<< "test0.jpg: "<<str<<endl;
    str = db_map.query("test1.jpg");
	cout<< "test1.jpg: "<<str<<endl;
    str = db_map.query("test2.jpg");
	cout<< "test2.jpg: "<<str<<endl;

}
