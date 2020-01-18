#pragma once
#ifndef _AUTO_FREE_H_
#define _AUTO_FREE_H_

#define AutoFree(className, instance) \
impl__AutoFree<className> _auto_free_##instance(&instance, false)
#define AutoFreeA(className, instance) \
impl__AutoFree<className> _auto_free_array_##instance(&instance, true)
template<class T>
class impl__AutoFree
{
private:
    T** ptr;
    bool is_array;
public:
    /**
     * auto delete the ptr.
     */
    impl__AutoFree(T** p, bool array) {
        ptr = p;
        is_array = array;
    }

    virtual ~impl__AutoFree() {
        if (ptr == NULL || *ptr == NULL) {
            return;
        }

        if (is_array) {
            delete[] *ptr;
        } else {
            delete *ptr;
        }

        *ptr = NULL;
    }
};

#endif//_AUTO_FREE_H_

