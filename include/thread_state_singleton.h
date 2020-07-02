#ifndef AVA_INCLUDE_THREAD_STATE_SINGLETON_H_
#define AVA_INCLUDE_THREAD_STATE_SINGLETON_H_

/**
 * This class ensures that the thread specific state instance gets destructed
 * as the thread exits.
 */
class ThreadStateSingletonDestroyer {
public:
  ~ThreadStateSingletonDestroyer();
};

class ThreadStateSingleton {
public:
  static ThreadStateSingleton* GetState();
  uint64_t GetThreadId() {
    return thread_id_;
  }

protected:
  ThreadStateSingleton(uint64_t thread_id) {
    thread_id_ = thread_id;
  }

private:
  static void DestroyState();
  static uint64_t thread_index_;  /* static TLS slot index */
  uint64_t thread_id_;

  friend ThreadStateSingletonDestroyer;
};

uint64_t ThreadStateSingleton::thread_index_ = -1;

ThreadStateSingletonDestroyer::~ThreadStateSingletonDestroyer() {
  ThreadStateSingleton::DestroyState();
}

void ThreadStateSingleton::DestroyState() {
  if (thread_index_ == -1)
    return;

  ThreadStateSingleton* state = 
}


#endif  // AVA_INCLUDE_THREAD_STATE_SINGLETON_H_
