#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

struct node {
    int val;
    node* next;
};

node* push_front(node* n, int val) {
    if (!n) {
        n = new node;
        n->val = val;
        n->next = nullptr;
        return n;
    }

    node* new_node = new node;
    new_node->val = val;
    new_node->next = n;
    return new_node;
}

void push_back(node* n, int val) {
    node* cursor = n;
    while (cursor->next != nullptr) {
        cursor = cursor->next;
    }

    node* new_node = new node;
    new_node->val = val;
    new_node->next = nullptr;
    cursor->next = new_node;
}

int main() {
    string left, right;
    cout << "Enter numbers: ";
    cin >> left >> right;

    node* a = nullptr;
    node* b = nullptr;

    for (size_t i = 0; i < max(left.size(), right.size()); i++) {
        if (i < left.size()) {
            a = push_front(a, left[i] - '0');
        } else {
            push_back(a, 0);
        }

        if (i < right.size()) {
            b = push_front(b, right[i] - '0');
        } else {
            push_back(b, 0);
        }
    }

    int carry = 0;
    node* c = nullptr;
    while (a) {
        int sum = a->val + b->val + carry;

        c = push_front(c, sum % 10);

        carry = sum / 10;

        node* a_tmp = a;
        node* b_tmp = b;
        a = a->next;
        b = b->next;

        delete a_tmp;
        delete b_tmp;
    }

    if (carry != 0) {
        c = push_front(c, carry);
    }

    while (c) {
        printf("%d", c->val);
        node* tmp = c;
        c = c->next;
        delete tmp;
    }
    printf("\n");
}
