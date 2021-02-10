#include <string>

int* gp; // will be pointing to address 0 (NULL)

struct Foo // 12 bytes
{
	int a;	// 4 bytes
	int b;	// 4 bytes
	int c;	// 4 bytes
	std::string cs;		// 24 bytes default constructor will init this to emptry string ""
	char* pcs;			// who keeps the storage for this string.
	Foo() {}
	~Foo() { free(pcs); }
	Foo(const Foo& f) :
		a(f.a),
		b(f.b),
		c(f.c),
		cs(f.cs)		// more efficient vs assiging later through assignment operator
	{ // cs will have been inited using default constructor if not using initizlizer

		cs = f.cs;
	}// Foo f, g;  Foo h = g; h = f; This function is called. I have to ensure to copy all members including cs.
	// assignment operator (copy after init of object) vs copy constructor (initialization of object)


	// Foo f; Foo g;  g = f;
	Foo operator=(const Foo& f)
	{
		a = f.a;
		b = f.b;
		c = f.c;
		cs = f.cs;		// cs was initalized already.  copy operation on string is happening.
	}


	/*String s;
	s = s1;*/

};


class String
{
	bool operator==(const String& s2);
	bool operator<(const String& s2);
	bool operator>(const String& s2);
	bool operator=(const String& s2);
};




void badfunc(Foo* p, char* str)		// assuming str is null terminated string
{
	p->cs = strdup(str);		// only pointer assignment. Foo strucutre does not have its own copy of the string.
						// when this function returns. We don't know but str may be freed. Then Foo structre has dangling pointer.
						// make own copy of contents to be safe
						// no need for strdup if using std::string.  copy constructor automatically allocates and copies contents.
	//p->cs = str;
	//p->a = 5;
}

void caller()
{
	// keep space for f and g here.   just allocate 72 bytes extra on stack.
	{// push in stack here would cost instructions. sub esp, 36
		Foo f;	// but call constructor here.
		{ // push in stack here would cost instructions. sub esp, 36
			Foo g;
			// f and g must be allocated on stack. But is it done at start of function itself?
		}	// stack unwind? destructor is called. unwinding would cost instrucitons. add esp, 36

	}	// stack unwind? destructor is called. unwinding would cost instrucitons. add esp, 36
	Foo f;
	char* s = strdup("hello");
	badfunc(&f, s);
	free(s);
	printf("%s", f.cs);		// problem? safe with strdup
							// but need to free memory
	free(f.pcs);				// no need for this if using std::string instead of char*
							// also no need if you define your own destructor that frees cs.
}	// f is destroyed by stack unwinding.  But f.cs has not been freed. memory leak.

// you can directly define before use.
// if you use before defining then declare first.
void func1(int a, int b, Foo* pf)
{
	Foo f;					// stack adds 12 bytes.  constructor is called in C++
	f.a = a;
	f.b = b;
	f.c = f.a + f.b;		// direct reference to memory for add instructions (ignore)

	if (a < 0)
	{
		// f and f1 both objects must exist
		Foo f1;
		*pf = f1;
		return;
	}

	*pf = f;			// will try to write to memory pointed by pf by the extent of 36 bytes. it better exist.
	return;				// copy entire contents of f to reutrn value.  will call constructor for the object being returned.
							// return value should be on stack?
							// simple values like int are returned in ax (16bit) / eax (32bit) / rax (64it) register.
} // stack unwinds on return and f is destroyed (in C++ destructor is called)

Foo* func(int a, int b)		// a and b are placed on stack before call so that func can access them.
								// when placed on stack they are copied
								// 4 byte of a is copied.  4 bytes of b is copied.
								// if passing a pointer.  The pointer is copied on stack.  Address is copied not the object.
								// if passing by reference.  Address is copied.  this is transparent to programmer.
{
	Foo* pf;				// stack adds 8(4 sometimes/32bit) bytes on stack for pointer
	pf = new Foo;			// allocate 12 bytes on heap and assign to pointer.  in C++ constructor would be called here.
							// constructor for s should be called.
							// use new not malloc to ensure constructor is called.
							// never freed until delete is explicitly called.
							// process exit does not return memory to heap.  But OS reclaims all memory used by process.

	// allocation reserves memory on heap.  usually somewhat expensive operation.
	// free blocks in heap are stored in some linked list. list node removal operation is involved.
	// heap of some size (few MBs) is created when process starts.
	// if heap runs out of space more is allocated from operations system (by making system calls).

	pf->a = a;
	pf->b = b;
	pf->c = pf->a + pf->b;		// same operation but instructions are different (costlier)

	return pf;


	Foo f;
	return &f;		// returning pointer to local variable which was on stack.
					// stack will unwind on return and the space will later be used for another functiona call
					//  for another local or parameter
					// not a good idea.
} // stack unwinds on return and f is destroyed (in C++ destructor is called)



// f1 is calling f2 and f2 is returning something.  What are legal options:
// 1. return a whole object (not pointer) - copied on stack and returned to f1
// 2. take a param that can be modifed (pointer to object / reference to object).  same thing. diff is using -> vs . in f2
// 3. allocate on heap and return address
// these options allow lifetime of returned information to be beyond lifetime of f2


// int, int -> int // return sum  params by copy and reutrn on register
//		passing by pointer would actually need 8 byte per int.  int themselves are just 4 bytes.
// big struct, big struct -> int // better to pass structure by pointer/ref irrespective of whether you want to modify it
//		if this were sent on stack by copy, copying would be costly.
//		good practice to delcare const if f2 intends to only read not modify.
//		compiler error if you modify object through const pointer





int main(int argc, char* argv[])
{
	Foo f1;
	func1(5, 6, &f1);  // create space on stack for return value structure

	Foo* pf = new Foo;
	func1(5, 6, pf);		// when this is called memory must have been secured for the Foo object.  Either on stack or heap or global.
	Foo* x = func(4, 5);
}

// memory allocation aspect
//    - stack (function parameters / function locals)
//    - heap (new / delete / malloc / free)
//    - globals (load time loader allocates)
// execution cost aspect
// life cycle aspect
