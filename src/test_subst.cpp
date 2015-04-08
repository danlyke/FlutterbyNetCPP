#include <map>
#include <string.h>
#include "fbyregex.h"

using namespace std;


int main(int /* argc */, char ** /* argv */, char ** /* env */)
{
    map<string,string> vars;
    vars["hello"] = string("is it me you're looking for?");
    vars["there"] = string("I can see it in your eyes");

    const char *str = "Well $hello, gorgeous, $there, dude!";
    cout << subst(str, strlen(str), vars) << endl;
}
