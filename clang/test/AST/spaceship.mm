// RUN: %clang_cc1 -triple x86_64-apple-macosx10.10 -std=c++20 -ast-dump %s | FileCheck %s

@interface MyObject : NSObject
@property (nonatomic, assign) int value;
@end

@implementation MyObject
@end

@interface SpaceshipOperatorTests : NSObject
@end

@implementation SpaceshipOperatorTests

- (BOOL)testSpaceshipOperator {
  MyObject *obj1 = [[MyObject alloc] init];
  obj1.value = 5;

  MyObject *obj2 = [[MyObject alloc] init];
  obj2.value = 10;

  auto result = [obj1 <=> obj2];
  return result;
}

@end
