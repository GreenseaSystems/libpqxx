#include <iostream>
#include <sstream>

#include <pqxx/all.h>
#include <pqxx/largeobject.h>

using namespace PGSTD;
using namespace pqxx;

namespace
{

const string Contents = "Large object test contents";

class CreateLargeObject : public Transactor
{
public:
  explicit CreateLargeObject(LargeObject &O) :
    Transactor("CreateLargeObject"),
    m_Object(),
    m_ObjectOutput(O)
  {
  }

  void operator()(argument_type &T)
  {
    LargeObjectAccess A(T);
    m_Object = LargeObject(A);
    cout << "Created large object #" << m_Object.id() << endl;
    A.write(Contents);
    A.to_file("pqxxlo.txt");
  }

  void OnCommit()
  {
    m_ObjectOutput = m_Object;
  }

private:
  LargeObject m_Object;
  LargeObject &m_ObjectOutput;
};


class DeleteLargeObject : public Transactor
{
public:
  explicit DeleteLargeObject(LargeObject O) : m_Object(O) {}

  void operator()(argument_type &T)
  {
    m_Object.remove(T);
  }

private:
  LargeObject m_Object;
};

}


// Test program for libpqxx: write large object to test files.
//
// Usage: test54 [connect-string]
//
// Where connect-string is a set of connection options in Postgresql's
// PQconnectdb() format, eg. "dbname=template1" to select from a database
// called template1, or "host=foo.bar.net user=smith" to connect to a
// backend running on host foo.bar.net, logging in as user smith.
int main(int, char *argv[])
{
  try
  {
    Connection C(argv[1]);

    LargeObject Obj;

    C.Perform(CreateLargeObject(Obj));
    C.Perform(DeleteLargeObject(Obj));
  }
  catch (const sql_error &e)
  {
    cerr << "SQL error: " << e.what() << endl
         << "Query was: '" << e.query() << "'" << endl;
    return 1;
  }
  catch (const exception &e)
  {
    cerr << "Exception: " << e.what() << endl;
    return 2;
  }
  catch (...)
  {
    cerr << "Unhandled exception" << endl;
    return 100;
  }

  return 0;
}


