



!-----
!BASIC
!-----

!
!Printing Tests

# These are just some simple printing tests
# since most of the test programs rely on
# printing to stdout these tests
# ensure that this facility works


blue basic/p001
--------------
1
--------------

blue basic/p002
--------------
a test string
--------------

blue basic/p003
--------------
1234567890
--------------

blue basic/p004
--------------
Hello world!

--------------

blue basic/p005
--------------
1Erik Lechak1
--------------

blue basic/convert.bl
--------------
123
123
123.456
123
-123
123.456
-123.456

---------------

!
!Number Tests

blue basic/number01
------------------
9
-3
0.5
18
729
3

9
-3
0.5
18
729
3

------------------



!
!Module Tests

blue basic/moduse
--------------
Hi

--------------

blue basic/moduse2
--------------
Hello5
--------------


!
!Attribute Tests

blue basic/attribs01
----------------------
3
z
y
x

-----------------------

blue basic/attreuse
------------------
3
3
3
3
0
0
0
0

------------------





!
!Object Pool Test

blue basic/poolfill
------------------
80done
80done
80done
80done
80done

------------------



!
!Refcount Tests

blue basic/rc01
-------------
1    1
1    1
1    1
1
1
1
1
1
1

-------------



!
!Mathematical Expression Tests


blue basic/m001
--------------
2
--------------

blue basic/m002
--------------
6
--------------

blue basic/m003
--------------
10
--------------

blue basic/m004
--------------
10
--------------

blue basic/m005
--------------
-1
--------------

blue basic/m006
--------------
-10
--------------

blue basic/m007
--------------
10
--------------

blue basic/m008
--------------
5
--------------

blue basic/m009
--------------
3.1415
--------------

blue basic/m010
--------------
2.5
--------------


!
!Conditional Tests



blue basic/c001
--------------
TF
--------------


blue basic/c002
--------------
0
TFF
FTT
TFF
TTF
FFT
FTT
FTT
TFF
FTT
FFT
TTF
TFF
1
FTF
TFT
FFF
TFF
FTT
TTT
TFT
FTF
TTT
FTT
TFF
FFF
-1
FFT
TTF
TTF
TTT
FFF
FFT
TTF
FFT
FFT
FFF
TTT
TTF
500
FFF
TTT
FFF
FFF
TTT
TTT
TTT
FFF
TTT
TTT
FFF
FFF
-500
FFF
TTT
TTT
TTT
FFF
FFF
TTT
FFF
FFF
FFF
TTT
TTT

--------------


blue basic/c003
--------------
FABCDEF
--------------


blue basic/c004
--------------
15
10
11
12
13
14
15

--------------

blue basic/c005
--------------
001
100
010

--------------


!
!Function Tests



blue basic/f001
--------------
AA
--------------

blue basic/f002
--------------
5051
--------------

blue basic/f003
--------------
ABC
--------------


blue basic/f004
--------------
5
--------------

blue basic/f005
--------------
101020
--------------

blue basic/f006
--------------
123459
--------------


blue basic/f007
--------------
1
--------------


blue basic/f008
--------------
hello
19
--------------

blue basic/f009
--------------
hello
--------------

blue basic/selftest
-----------------
2
4
2
2
2

------------------


!
!Trap Tests



blue basic/t001
--------------

1
12
123
1234

1
12
123
1234

--------------

!
!Lexical Tests

blue basic/lex
----------------
21

----------------

blue basic/lex2
----------------
g1  101
g1  102
g2  103
g2  104
g1  105
g1  201
g2  202
g2  106
g1  107

----------------

blue basic/lex3
----------------
10
9
8
7
6
5
4
3
2
1

----------------

blue basic/lex4
----------------
45

----------------

blue basic/lex5
----------------
I am Kippy age 12
I am Kippy age 16

----------------

blue basic/lex6
----------------
13
25
37

----------------

!
!Backtrace Tests

blue backtrace/c01
---------------
VariableUsedBeforeSet
 [module:backtrace/c01   line:6]
 [module:backtrace/c01   line:9]

---------------


blue backtrace/c02
---------------
hello
 [module:backtrace/c02   line:5]
 [module:backtrace/c02   line:8]

---------------

!
!String Tests




blue basic/s001
------------------
0123456789
10
01234567890123456789
2
StringFindFailed
1
0
0
1
0
56789
56
23456
2345
89

-------------------


!
!Arguments

blue basic/args01 a b c
--------------------------
basic/args01
a
b
c
Finished

---------------------------



blue basic/args02
---------------------------

1
12
123
1234

1
12
123
1234

---------------------------

blue basic/args03
---------------------------
9
-10
-11
-9
20
0

---------------------------


blue basic/args04
---------------------------
20
---------------------------

blue basic/args04 105
---------------------------
105
---------------------------

blue basic/args04 1.457
---------------------------
1.457
---------------------------


blue basic/arg01
----------------------
Hello

----------------------

blue basic/arg02
----------------------
output:
one
two
three

----------------------



!
!Arrays

blue basic/a001
---------------------------
4
3
7

---------------------------


blue basic/a002
---------------------------
1
2
3
4
6
7
8
9

---------------------------



blue basic/a003
---------------------------
10
20
30
0
30
0
1
2
3
4
5
6

---------------------------


blue basic/sort
----------------------
[hello,1,2,3,4,5]
-----------------------

blue basic/sort2
------------------
[5,4,3,2,1]
------------------


!
!-------
!Strings
!-------

blue strings/stringadd.bl
-----------------------------
abcdefhijklm
-----------------------------


blue strings/find.bl
---------------------------
7

---------------------------

blue strings/trim.bl
--------------------
     abcdef    A
ltrim:
abcdef    A
  abcdef    A
abcdef    A
     abcdef    A
rtrim:
     abcdefA
     abcdef A
     abcdefA
     abcdef    A
trim:
abcdefA
  abcdef A
abcdefA
     abcdef    A
     abcdeA
     abcdefA
     abcdef   A
     abcdefA
abcdef    A
 abcdef    A
    abcdef    A
abcdef    A
2233443322A
334433A

--------------------


blue strings/replace.bl
---------------------------
abcd
123bcd
123cd
123d
123
abcdabcdabcd
123bcd123bcd123bcd
123cd123cd123cd
123d123d123d
123123123

---------------------------


blue strings/split.bl
---------------------------
<hello><there><how><are><you>

---------------------------

blue strings/stringtest.bl
---------------------------
2345
e

---------------------------

blue strings/substring.bl
---------------------------
56789
56
23456
2345
89
Caught: StrSubstrStartIndexInvalid 

---------------------------

blue strings/repeat.bl
----------------------
abcabcabcabcabc
----------------------





!
!-------
!Plugins
!-------


!
!Dictionaries

blue basic/d001
---------------------------
3

---------------------------

blue basic/d002
---------------------------
2
erik
ChildNotFound
ChildNotFound

---------------------------


blue basic/d003
---------------------------
3
c
b
a

---------------------------

blue basic/d004
---------------------------
17
19
2
20
6

---------------------------

blue plugins/dict.bl
---------------------------
AssignError2hello
---------------------------



!
!Types
blue plugins/types.bl
-------------------------
Creating a dog named Buddy
Creating a dog named Kippy

Chomp, Chomp
Chomp, Chomp

Buddy says: Woof, woof!
Kippy says: Woof, woof!

Buddy says: Woof, woof!
Kippy says: Yipe, Yipe!

Dog named Buddy has left the building
Dog named Kippy has left the building

-------------------------

blue basic/ampand.bl
------------------
hello
types match
types dont match

-----------------------

blue basic/ampand2.bl
------------------
hello
hello
hello
hello
hello
hello
hello

-----------------------

blue basic/ampand3.bl
------------------
constructor
destructor

-----------------------


!
!Streams
blue plugins/memstream.bl
----------------------
Hello
 there
 how are

----------------------


!
!Lists

blue plugins/lists.bl
-------------------------
35Erik7

-------------------------

!
!XML

blue plugins/xml.bl
-------------------------
START <erik>  .erik
START <kip>  .erik.kip
CHARS 'how '
START <donna>  .erik.kip.donna
END <donna>  .erik.kip.donna
CHARS 'are you'
END <kip>  .erik.kip
END <erik>  .erik

-------------------------



!
!Threads

blue plugins/threads.bl
-------------------------
10000

-------------------------






!
!-------
!demos
!-------


blue demos/args.bl  hello there
-------------------------
demos/args.bl
hello
there
Finished

-------------------------

blue demos/argtrap.bl
-------------------------

1
12
123
1234

1
12
123
1234

-------------------------


blue demos/arraytest.bl
-------------------------
0

5
3

3
8

8
3

1 hello
2 hello
3 hello
4 hello
5 hello
6 hello
7 hello
8 hello
1 hello
2 hello
3 hello
4 hello
5 hello
6 hello
7 hello
8 hello
1 hello,2 hello,3 hello,4 hello,5 hello,6 hello,7 hello,8 hello

-------------------------

blue demos/maptest.bl
------------------------
Even:
0
2
4
6
8
Odd:
1
3
5
7
9

------------------------

blue demos/withtest.bl
-------------------------
1 + 4
5
-------------------------


blue demos/class.bl
-------------------------
A dog named Kippy was Created
woof woof.  My name is Kippy
yipe yipe.  My name is Kippy
woof woof.  My name is Kippy
Kippy ran away

-------------------------

blue demos/clone.bl
-------------------------
bark
creating a Dog
bark
Hello
Hello
destroying a Dog

-------------------------

blue demos/contract.bl
-------------------------
Kippy: [name,speak]
woof woof.  My name is Kippy
Buddy: [age,name,speak,tellAge]
woof woof.  My name is Buddy
my age is: 15
Unknown: [age,tellAge]

-------------------------


blue demos/asattrib.bl
-------------------------
1234
-------------------------


blue demos/copy.bl
-------------------------
Hello[x,y,z]
Hello[x,y,z]
Hello[]

-------------------------

blue demos/del.bl
-------------------------
4
a not found

-------------------------


blue demos/extend.bl
-------------------------
woof woof.  My name is Kippy
woof woof.  My name is Buddy

-------------------------


blue demos/foreach.bl
-------------------------
Hello erik
Hello donna
Hello kippy

-------------------------

blue demos/constructor.bl
-------------------------
I am an animal
A dog named Kippy was Created
woof woof.  My name is Kippy
yipe yipe.  My name is Kippy
woof woof.  My name is Kippy
Kippy ran away

-------------------------




!
!-------
!Benchmarks
!-------

blue benchmarks/ackermann 3 3
---------------------------
61
------------------------


blue benchmarks/erathos 2
---------------------------
8193  1028

------------------------


blue benchmarks/fib 7
---------------------------
8

------------------------


blue benchmarks/func 1
---------------------------
done
------------------------


blue benchmarks/loop 1
---------------------------
done
------------------------


blue benchmarks/tak 5
---------------------------
10

------------------------






