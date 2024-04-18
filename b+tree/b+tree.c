#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BNODE_INTERNAL 1
#define BNODE_LEAF 2

#define HEADER 4
#define BTREE_PAGE_SIZE 4096
#define BTREE_MAX_KEY_SIZE 1000
#define BTREE_MAX_VAL_SIZE 3000

typedef struct fat_ptr {
  size_t len;
  uint8_t *data;
} fat_ptr;

typedef struct bnode {
  uint8_t *data;
} bnode;

typedef struct btree {
  uint64_t root;
  // get for deref
  // new for new page
  // del dealloc page
} btree;

void btree_init(void) {
  uint64_t node1max =
      HEADER + 8 + 2 + 4 + BTREE_MAX_KEY_SIZE + BTREE_MAX_VAL_SIZE;
  assert(node1max < BTREE_PAGE_SIZE);
}

uint16_t bnode_type(bnode *bn) { return *(uint16_t *)bn->data; }

uint16_t bnode_nkeys(bnode *bn) { return *(uint16_t *)(bn->data + 1); }

void bnode_set_header(bnode *bn, uint16_t type, uint16_t nkeys) {
  *(uint16_t *)bn->data = type;
  *(uint16_t *)(bn->data + 2) = nkeys;
}

uint64_t bnode_get_ptr(bnode *bn, uint16_t idx) {
  assert(idx < bnode_nkeys(bn));
  uint64_t pos = HEADER + (8 * idx);
  return *(uint64_t *)(bn->data + pos);
}

void bnode_set_ptr(bnode *bn, uint16_t idx, uint16_t val) {
  assert(idx < bnode_nkeys(bn));
  uint64_t pos = HEADER + (8 * idx);
  *(uint64_t *)(bn->data + pos) = val;
}

// Where is this offset in our offset list
uint16_t bnode_offset_pos(bnode *bn, uint16_t idx) {
  assert(1 <= idx && idx <= bnode_nkeys(bn));
  return HEADER + (8 * bnode_nkeys(bn)) + (2 * (idx - 1));
}

uint16_t bnode_get_offset(bnode *bn, uint16_t idx) {
  if (idx == 0) {
    return 0;
  }
  return *(uint16_t *)(bn->data + bnode_offset_pos(bn, idx));
}

void bnode_set_offset(bnode *bn, uint16_t idx, uint16_t offset) {
  *(uint16_t *)(bn->data + bnode_offset_pos(bn, idx)) = offset;
}

uint16_t bnode_kv_pos(bnode *bn, uint16_t idx) {
  assert(idx <= bnode_nkeys(bn));
  return HEADER + (8 * bnode_nkeys(bn)) + (2 * bnode_nkeys(bn)) +
         bnode_get_offset(bn, idx);
}

// returned key must be freed
fat_ptr bnode_get_key(bnode *bn, uint16_t idx) {
  assert(idx < bnode_nkeys(bn));
  uint64_t pos = bnode_kv_pos(bn, idx);
  uint16_t key_length = *(uint16_t *)(bn->data + pos);
  uint8_t *key = bn->data + pos + 4;
  return (fat_ptr){.len = key_length, .data = key};
}

fat_ptr bnode_get_val(bnode *bn, uint16_t idx) {
  assert(idx < bnode_nkeys(bn));
  uint64_t pos = bnode_kv_pos(bn, idx);
  uint16_t key_length = *(uint16_t *)(bn->data + pos);
  uint16_t val_length = *(uint16_t *)(bn->data + pos + 2);
  uint8_t *val = bn->data + pos + 4 + key_length;
  return (fat_ptr){.len = val_length, .data = val};
}

// node size in bytes
uint16_t bnode_nbytes(bnode *bn) { return bnode_kv_pos(bn, bnode_nkeys(bn)); }

uint16_t bnode_lookup_le(bnode *bn, fat_ptr key) {
  uint16_t nkeys = bnode_nkeys(bn);
  uint16_t found = 0;
  // The first key is a copy from the prent node,
  // thus it's alwaysss than or equal to the key
  for (uint16_t i = 1; i < nkeys; i++) {
    int compare = memcmp(bnode_get_key(bn, i).data, key.data, key.len);
    if (compare <= 0) {
      found = i;
    }
    if (compare >= 0) {
      break;
    }
  }

  return found;
}

void bnode_append_range(bnode *new, bnode *old, uint16_t dest_new,
                        uint16_t src_old, uint16_t n) {
  assert(src_old + n <= bnode_nkeys(old));
  assert(dest_new + n <= bnode_nkeys(new));
  if (n == 0) {
    return;
  }

  // Pointers
  for (uint16_t i = 0; i < n; i++) {
    bnode_set_ptr(new, dest_new + i, bnode_get_ptr(old, src_old + 1));
  }
  // Offsets
  uint16_t dest_begin = bnode_get_offset(new, dest_new);
  uint16_t src_begin = bnode_get_offset(old, src_old);
  for (uint16_t i = 1; i <= n; i++) { // the range is [1,n]
    uint16_t offset =
        dest_begin + bnode_get_offset(old, src_old + i) - src_begin;
    bnode_set_offset(new, dest_new + i, offset);
  }
  // KVs
  uint16_t begin = bnode_kv_pos(old, src_old);
  uint16_t end = bnode_kv_pos(old, src_old + n);
  memcpy(new->data + bnode_kv_pos(new, dest_new), old->data + begin,
         end - begin);
}

void bnode_append_kv(bnode *new, uint16_t idx, uint64_t ptr, fat_ptr key,
                     fat_ptr val) {
  // pointers
  bnode_set_ptr(new, idx, ptr);
  // KVs
  uint16_t pos = bnode_kv_pos(new, idx);
  *(uint16_t *)(new->data + pos) = key.len;
  *(uint16_t *)(new->data + pos + 2) = val.len;
  memcpy(&new->data[pos + 4], key.data, key.len);
  memcpy(&new->data[pos + 4 + key.len], val.data, val.len);
  // Offset of the new key
  bnode_set_offset(new, idx + 1,
                   bnode_get_offset(new, idx) + 4 + key.len + val.len);
}

void bnode_leaf_insert(bnode *new, bnode *old, uint16_t idx, fat_ptr key,
                       fat_ptr val) {
  bnode_set_header(new, BNODE_LEAF, bnode_nkeys(old) + 1);
  bnode_append_range(new, old, 0, 0, idx);
  bnode_append_kv(new, idx, 0, key, val);
  bnode_append_range(new, old, idx + 1, idx, bnode_nkeys(old) - idx);
}

// Insert a KV into a node, the result miht be split into 2 node
// the caller is responsible for deallocation the input node
// and splitting and allocating result nodes
bnode tree_insert(btree *tree, bnode node, fat_ptr key, fat_ptr val) {
  // the result node
  // it's allowed to be bigger than 1 page and will be split if so
  bnode new = {.data = malloc(2 * BTREE_PAGE_SIZE)};

  // where to insert the key?
}

void bnode_dump(bnode *bn) {
  for (int i = 0; i < 128; i++) {
    if (i % 16 == 0 && i != 0) {
      printf("\n");
    }
    printf("%02d ", bn->data[i]);
  }
}
/* | type | nkeys |  pointers  |   offsets  | key-values */
/* |  2B  |   2B  | nkeys * 8B | nkeys * 2B | ... */
/* | klen | vlen | key | val | */
/* |  2B  |  2B  | ... | ... | */

int main() {
  bnode b = {.data = calloc(4096, 1)};
  bnode_set_header(&b, BNODE_LEAF, 72);
  bnode_set_ptr(&b, 0, 69);
  printf("Node type: %d\n", bnode_type(&b));
  bnode_dump(&b);
  free(b.data);
}
