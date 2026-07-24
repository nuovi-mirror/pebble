pub const tests = 
    \\New msg "msg"
    \\New ARG1 "msg"
    // print test
    \\New msg "Starting std/Print test..."
    \\Escape Print
    \\New msg "std/Print Test passed"
    \\Escape Print
    \\New msg "std/Print test ended"
    \\Escape Print
    // input test
//    \\New msg 'Starting std/Input test...'
//    \\Escape Print
//    \\New msg 'Press any key'
//    \\Escape Print
//    \\New msg 'The key should be echoed'
//    \\Escape Print
//    \\New msg 'after you press Enter'
//    \\Escape Print
//    \\New ARG1 'input'
//    \\Escape Input
//    \\New msg input
//    \\Escape Print
//    \\New ARG1 'msg'
//    \\New msg 'If the input was echoed,'
//    \\Escape Print
//    \\New msg 'then the test passed.'
//    \\Escape Print
//    \\New msg 'std/Input test ended'
//    \\Escape Print
    // math test
    \\New msg 'Starting vm/Expr (math / string ops) test...'
    \\Escape Print
    \\New msg '2 should be printed 4 times'
    \\Escape Print
    \\New msg '1 should be printed 6 times'
    \\Escape Print
    \\New msg 'abcd should be printed 3 times'
    \\Escape Print
    \\New msg "1 + 1"
    \\Escape Print
    \\New msg "2 / 1"
    \\Escape Print
    \\New msg "3 - 1"
    \\Escape Print
    \\New msg "1 * 2"
    \\Escape Print
    \\New msg "2 == 1"
    \\Escape Print
    \\New msg "2 != 2"
    \\Escape Print
    \\New msg "e ?= a"
    \\Escape Print
    \\New msg "ab e?= e"
    \\Escape Print
    \\New msg "ab s?= e"
    \\Escape Print
    \\New msg "ab -?= e"
    \\Escape Print
    \\New msg "ab s++ cd"
    \\Escape Print
    \\New ARG1 'str'
    \\New str 'abcdd'
    \\New ARG2 'numb'
    \\New numb '4'
    \\Escape StrSplitNumL
    \\New numb '1'
    \\New str 'aabcd'
    \\Escape StrSplitNumR
    \\New ARG1 'strLeft'
    \\Escape Print
    \\New ARG1 'strRight'
    \\Escape Print
    \\New ARG1 'msg'
    \\New msg 'vm/Expr test ended'
    \\Escape Print
    // Literals test
    \\New msg 'Starting vm/Literals test...'
    \\Escape Print
    \\New msg 'A mathematical expression should be printed'
    \\Escape Print
    \\New str '5 + 3'
    \\New msg str
    \\Escape Print
    \\New msg 'The result of the previous expression should be printed'
    \\Escape Print
    \\New msg {str}
    \\Escape Print
    \\New msg 'vm/Literals test ended'
    \\Escape Print
    // functions test
    \\New msg 'Starting vm/Func test...'
    \\Escape Print
    \\Func testfunc
    \\New msg 'vm/Func test passed'
    \\Escape Print
    \\End
    \\Call testfunc
    \\New msg 'vm/Func test ended'
    \\Escape Print
    // if test
    \\New msg 'Starting vm/If test...'
    \\Escape Print
    \\New msg 'A trigger statement should'
    \\Escape Print
    \\New msg 'be printed ONCE'
    \\Escape Print
    \\Func testfunc2
    \\New msg 'vm/If condition triggered'
    \\Escape Print
    \\End
    \\If testfunc2 "1 == 1"
    \\If testfunc2 "1 == 2"
    \\New msg 'vm/If test ended'
    \\Escape Print
    // return test
    \\New msg 'Starting vm/Return test...'
    \\Escape Print
    \\New msg 'Assume pass unless stated otherwise'
    \\Escape Print
    \\Func testfunc3
    \\Return
    \\New msg 'vm/Return test failed'
    \\Escape Print
    \\End
    \\Call testfunc3
    \\New msg 'vm/Return test ended'
    \\Escape Print
    // embedding test
    \\New msg 'Starting tbdlang code embedding test...'
    \\Escape Print
    \\New msg 'Obviously it passed, since this code is'
    \\Escape Print
    \\New msg 'embedded into the vm already'
    \\Escape Print
    \\New msg 'tbdlang code embedding test ended'
    \\Escape Print
    // execution test
    \\New msg 'Starting tbdlang execution test...'
    \\Escape Print
    \\New msg 'This test program is written in tbdlang'
    \\Escape Print
    \\New msg 'This program is running...'
    \\Escape Print
    \\New msg 'So I would say that passes.'
    \\Escape Print
    \\New msg 'tbdlang execution test ended'
    \\Escape Print
    // random test
    \\New msg 'Starting std/Random test...'
    \\Escape Print
    \\New msg 'Three different numbers should be printed'
    \\Escape Print
    \\Escape Random
    \\New msg random
    \\Escape Print
    \\Escape Random
    \\New msg random
    \\Escape Print
    \\Escape Random
    \\New msg random
    \\Escape Print
    \\New msg 'std/Random test ended'
    \\Escape Print
    // proc test
//    \\New msg 'Starting std/Proc test...'
//    \\Escape Print
//    \\New msg 'This test requires a POSIX echo'
//    \\Escape Print
//    \\New ARG1 'procArgs'
//    \\New procArgs 'echo std/Proc/SpawnProc Passed'
//    \\Escape SpawnProc
//    \\New ARG1 'pid'
//    \\Escape WaitPid
//    \\New ARG1 'msg'
//    \\New msg 'If the message comes in late, then'
//    \\Escape Print
//    \\New msg 'std/Proc/WaitPid is broken.'
//    \\Escape Print
//    \\New msg 'std/Proc test ended'
//    \\Escape Print
    // end tests
    \\New msg "Testing completed."
    \\Escape Print
;

