//> Chunks of Bytecode memory-c
#include <stdlib.h>

//> Garbage Collection memory-include-compiler
#include "compiler.h"
//< Garbage Collection memory-include-compiler
#include "memory.h"
//> Strings memory-include-vm
#include "vm.h"
//< Strings memory-include-vm
//> Garbage Collection debug-log-includes

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif
//< Garbage Collection debug-log-includes
//> Garbage Collection heap-grow-factor

#define GC_HEAP_GROW_FACTOR 2
//< Garbage Collection heap-grow-factor

static uint8_t* heap;
static uint8_t* fromSpace;
static uint8_t* toSpace;

static uint8_t* allocPtr;
static uint8_t* scanPtr;

static size_t heapSize;

void initHeap(size_t size) {
  heapSize = size;

  heap = (uint8_t*)malloc(size);
  fromSpace = heap;
  toSpace = heap + size / 2;

  allocPtr = fromSpace;
}

static Obj* copy(Obj* obj) {
  if (obj == NULL) return NULL;

  if (obj->forwarding != NULL) {
    return obj->forwarding;
  }

  size_t size = sizeOf(obj);

  Obj* newObj = (Obj*)allocPtr;
  memcpy(newObj, obj, size);
  allocPtr += size;

  obj->forwarding = newObj;

  return newObj;
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) return NULL;

  if (allocPtr + newSize > fromSpace + heapSize / 2) {
    collectGarbage();
  }

  void* result = allocPtr;
  allocPtr += newSize;
  return result;
}

//> Garbage Collection mark-object
void markObject(Obj* object) {
  if (object == NULL) return;
//> check-is-marked
  if (object->isMarked) return;

//< check-is-marked
//> log-mark-object
#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void*)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif

//< log-mark-object
  object->isMarked = true;
//> add-to-gray-stack

  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj**)realloc(vm.grayStack,
                                  sizeof(Obj*) * vm.grayCapacity);
//> exit-gray-stack

    if (vm.grayStack == NULL) exit(1);
//< exit-gray-stack
  }

  vm.grayStack[vm.grayCount++] = object;
//< add-to-gray-stack
}
//< Garbage Collection mark-object
//> Garbage Collection mark-value
void markValue(Value value) {
  if (IS_OBJ(value)) markObject(AS_OBJ(value));
}
//< Garbage Collection mark-value
//> Garbage Collection mark-array
static void markArray(ValueArray* array) {
  for (int i = 0; i < array->count; i++) {
    markValue(array->values[i]);
  }
}
//< Garbage Collection mark-array
//> Garbage Collection blacken-object
static void scanObject(Obj* obj) {
  switch (obj->type) {
    case OBJ_STRING:
      break;

    case OBJ_FUNCTION: {
      ObjFunction* fn = (ObjFunction*)obj;
      fn->name = copy(fn->name);
      break;
    }

    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)obj;
      closure->function = copy(closure->function);

      for (int i = 0; i < closure->upvalueCount; i++) {
        closure->upvalues[i] = copy(closure->upvalues[i]);
      }
      break;
    }

    // handle all object types here...
  }
}
//< Garbage Collection blacken-object
//> Strings free-object
static void freeObject(Obj* object) {
//> Garbage Collection log-free-object
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void*)object, object->type);
#endif

//< Garbage Collection log-free-object
  switch (object->type) {
//> Methods and Initializers free-bound-method
    case OBJ_BOUND_METHOD:
      FREE(ObjBoundMethod, object);
      break;
//< Methods and Initializers free-bound-method
//> Classes and Instances free-class
    case OBJ_CLASS: {
//> Methods and Initializers free-methods
      ObjClass* klass = (ObjClass*)object;
      freeTable(&klass->methods);
//< Methods and Initializers free-methods
      FREE(ObjClass, object);
      break;
    } // [braces]
//< Classes and Instances free-class
//> Closures free-closure
    case OBJ_CLOSURE: {
//> free-upvalues
      ObjClosure* closure = (ObjClosure*)object;
      FREE_ARRAY(ObjUpvalue*, closure->upvalues,
                 closure->upvalueCount);
//< free-upvalues
      FREE(ObjClosure, object);
      break;
    }
//< Closures free-closure
//> Calls and Functions free-function
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      freeChunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }
//< Calls and Functions free-function
//> Classes and Instances free-instance
    case OBJ_INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      freeTable(&instance->fields);
      FREE(ObjInstance, object);
      break;
    }
//< Classes and Instances free-instance
//> Calls and Functions free-native
    case OBJ_NATIVE:
      FREE(ObjNative, object);
      break;
//< Calls and Functions free-native
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
//> Closures free-upvalue
    case OBJ_UPVALUE:
      FREE(ObjUpvalue, object);
      break;
//< Closures free-upvalue
  }
}
//< Strings free-object
//> Garbage Collection mark-roots
static void copyRoots() {
  // Stack
  for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
    if (IS_OBJ(*slot)) {
      *slot = OBJ_VAL(copy(AS_OBJ(*slot)));
    }
  }

  // Globals
  for (int i = 0; i < vm.globals.capacity; i++) {
    Entry* entry = &vm.globals.entries[i];
    if (entry->key != NULL) {
      entry->key = copy(entry->key);
      entry->value = OBJ_VAL(copy(AS_OBJ(entry->value)));
    }
  }

  // Open upvalues, call frames, etc. (same as markRoots)
}
//< Garbage Collection mark-roots
//> Garbage Collection trace-references
static void traceReferences() {
  while (vm.grayCount > 0) {
    Obj* object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}
//< Garbage Collection trace-references
//> Garbage Collection sweep
static void sweep() {
  Obj* previous = NULL;
  Obj* object = vm.objects;
  while (object != NULL) {
    if (object->isMarked) {
//> unmark
      object->isMarked = false;
//< unmark
      previous = object;
      object = object->next;
    } else {
      Obj* unreached = object;
      object = object->next;
      if (previous != NULL) {
        previous->next = object;
      } else {
        vm.objects = object;
      }

      freeObject(unreached);
    }
  }
}
//< Garbage Collection sweep
//> Garbage Collection collect-garbage
void collectGarbage() {
  // Swap spaces
  uint8_t* temp = fromSpace;
  fromSpace = toSpace;
  toSpace = temp;

  allocPtr = toSpace;
  scanPtr = toSpace;

  // 1. Copy roots
  copyRoots();

  // 2. Scan copied objects
  while (scanPtr < allocPtr) {
    Obj* obj = (Obj*)scanPtr;
    scanPtr += sizeOf(obj);

    scanObject(obj); // replaces blackenObject
  }
}
//< Garbage Collection collect-garbage
//> Strings free-objects
void freeObjects() {
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }
//> Garbage Collection free-gray-stack

  free(vm.grayStack);
//< Garbage Collection free-gray-stack
}
//< Strings free-objects
