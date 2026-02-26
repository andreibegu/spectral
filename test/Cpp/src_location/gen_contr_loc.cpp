// clang-format off
// Test that the source-location are correct in generated contracts
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

class AClass {
    public:
    int i = 0;

    /*@
    contract for generated src AClass;
    ensures this->i == 0;
    @*/
};


bool bar () {
    AClass c;
    c.i = 1;
    return c.i + 41;
}

// COM: Contract is attached to constructor
// define linkonce_odr dso_local void @_ZN11ClassyClassC2Ev(ptr noundef nonnull align 8 dereferenceable(48) %0)
// CHECK: define {{.*}} @_ZN6AClassC2Ev(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: MD-Nodes of the contract have location-info 
// CHECK-DAG: [[contractMD]] = !{[[contrLoc:![0-9]+]], i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]]}
// CHECK-DAG: [[contrLoc]] = !{!"pallas.srcLoc", i64 9, i64 5, i64 12, i64 5, [[fileMD:![0-9]+]]}

// CHECK-DAG: [[reqMD]] = !{!"pallas.ensures", [[ensLoc:![0-9]+]], 
// CHECK-DAG: [[ensLoc]] = !{!"pallas.srcLoc", i64 11, i64 5, i64 11, i64 25, [[fileMD]]}

// CHECK-DAG: [[fileMD]] = !DIFile(filename: "{{.*}}gen_contr_loc.cpp",