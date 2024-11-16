#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Plugin {
    char name[65];         // Given by plugin designer
    int (*init)(void);     // Init function to call when plugin is loaded, 0 on
                           // success otherwise error code
    void (*fini)(void);    // Called when plugin is unloaded
    int (*cmd)(char* str); // true (1) mean success no need for others, 0
                           // unsuccessful so try others
} Plugin;

typedef struct Node {
    Plugin* p;
    void* handle;
    struct Node* next;
} Node;

Node* head = NULL;

void ll_add(Plugin* p, void* handle) {
    Node* new = malloc(sizeof(Node));
    if (!new) {
        perror("malloc");
        return;
    }
    new->p = p;
    new->handle = handle;
    new->next = NULL;
    if (!head) {
        head = new;
        return;
    }

    new->next = head;
    head = new;
    return;
}

Node* ll_remove(const char* name) {
    if (!head) {
        return NULL;
    }

    if (!strcmp(head->p->name, name)) {
        Node* ret_val = head;
        head = NULL;

        return ret_val;
    }

    Node* cursor = head;
    while (head->next) {
        if (!strcmp(cursor->next->p->name, name)) {
            Node* to_delete = cursor->next;
            cursor->next = cursor->next->next;

            return to_delete;
        }
    }

    return NULL;
}

int main(void) {
    while (true) {
        printf("> ");
        char input_buffer[256] = {0};
        char* ret_val = fgets(input_buffer, sizeof(input_buffer), stdin);
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        if (!ret_val) {
            return 0;
        }

        if (!strcmp("quit", input_buffer)) {
            Node* cursor = head;
            while (cursor) {
                cursor->p->fini();
                Node* tmp = cursor->next;
                dlclose(cursor->handle);
                free(cursor);
                cursor = tmp;
            }

            return 0;
        } else if (!strncmp("load", input_buffer, 4)) {
            char* path_to_load = input_buffer + 5;

            void* plugin = dlopen(path_to_load, RTLD_LAZY);
            if (!plugin) {
                perror("dlopen");
                continue;
            }

            Plugin* p = dlsym(plugin, "export");
            if (!p) {
                printf("%s\n", dlerror());
                continue;
            }
            if (p->init() != 0) {
                // Probably something a bit more to do here
                return 1;
            }

            ll_add(p, plugin);
        } else if (!strncmp("unload", input_buffer, 6)) {
            char* name_to_unload = input_buffer + 7;

            Node* n = ll_remove(name_to_unload);
            if (!n) {
                continue;
            }

            dlclose(n->handle);
            free(n);
        } else if (!strncmp("list", input_buffer, 4) ||
                   !strncmp("plugins", input_buffer, 7)) {
            Node* cursor = head;
            while (cursor) {
                printf("%s\n", cursor->p->name);
                cursor = cursor->next;
            }
        } else {
            int code = 0;
            Node* cursor = head;
            while (code == 0 && cursor) {
                code = cursor->p->cmd(input_buffer);
                cursor = cursor->next;
            }
        }
    }
    Node* cursor = head;
    while (cursor) {
        cursor->p->fini();
        Node* tmp = cursor->next;
        dlclose(cursor->handle);
        free(cursor);
        cursor = tmp;
    }
}
