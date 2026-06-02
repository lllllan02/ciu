#include "binary_search.h"

int binary_search(int *arr, int len, int target) {
    int l = 0, r = len - 1, mid = 0;

    while (l <= r) {
        mid = l + (r - l) / 2;

        if (target > arr[mid]) {
            l = mid + 1;
        } else if (target < arr[mid]) {
            r = mid - 1;
        } else {
            return mid;
        }
    }

    return -1;
}

int binary_search_recur(int *arr, int l, int r, int target) {
    if (l > r) {
        return -1;
    }

    int mid = l + (r - l) / 2;

    if (target > arr[mid]) {
        return binary_search_recur(arr, mid + 1, r, target);
    } else if (target < arr[mid]) {
        return binary_search_recur(arr, l, mid - 1, target);
    }
    
    return mid;
}