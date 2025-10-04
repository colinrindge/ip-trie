// 
// File: trie.c 
// Starter trie module for an integer-keyed trie ADT data type
// @author CS@RIT.EDU
// @author Colin Rindge cfr1524
// // // // // // // // // // // // // // // // // // // // // // // // 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "trie.h"

/// Node_s: binary tree node containing a pointer to the left and right nodes and an entry containing the data within it.
/// Node is a pointer to a dynamically allocated struct Node_s.
/// NodeH is a pointer to a pointer to a struct Node_s.


typedef
struct Node_s {
    struct Node_s * left;   //< A pointer to the left node
    Entry data;             //< A struct containing the data      
    struct Node_s * right;  //< A pointer to the right node
    unsigned int height;
} * Node;

typedef Node * NodeH;          ///< handle to a trie node

/// Trie_s: The main trie struct, containing the root, a function pointer for printing entries, 
/// a function pointer for freeing values, the height of the tree, the total number of leaf nodes,
/// and the toal number of internal nodes
struct Trie_s {
    Node root; // The node of the top root of the tree
    void (*print_values)(Entry entry, FILE * stream); // A function that prints out the entries in the trie
    void (*free_value)(Value v); // A function that frees all contents of value struct, NULL if not necessary
    long int height; // The height of the tree
    size_t size; // The number of leaf nodes in the tree
    size_t node_count; // The number of internal nodes in the tree
};

// global constants for bit operations and sizes
// used by the application program

const size_t BITSPERBYTE = 8;
const size_t BITSPERWORD = 32;
const size_t BYTESPERWORD = 4;
const size_t RADIX = 256;

/*
 * Creates an Entry to be stored in a node of the trie using the passed values
 * 
 * @pre                     IP must already be converted to an integer equivalent
 * @param ip                The ip (key)
 * @param v                 The value struct
 */
Entry create_entry(ikey_t ip, Value v){    
    Entry new_entry = malloc(sizeof(struct Entry_s));

    if(new_entry == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_entry->key = ip;
    new_entry->v = v;

    return new_entry;
}

// Creates an empty Trie and returns a pointer to it, or NULL on failure
Trie ibt_create( void (*print_values)(Entry entry, FILE * stream) , void (*free_value)(Value v)){
    
    Trie trie = malloc(sizeof(struct Trie_s));
    if(trie != NULL){
        trie->root = NULL;
        trie->height = 0;
        trie->print_values = print_values;
        trie->free_value = free_value;
        trie->node_count = 0;
        trie->size = 0;
    }

    return trie;
}

/*
 * Frees all nodes and their data in a trie
 *
 * @param node      The root of the tree to free
 */
void free_nodes(Trie tr, Node node){
    if(node == NULL){
        return;
    }
    if(node->left != NULL){
        free_nodes(tr, node->left);
    }
    if(node->right != NULL){
        free_nodes(tr, node->right);
    }
    if(node->data != NULL){
        if(tr->free_value != NULL){
            (tr->free_value)(node->data->v);
        }
        free(node->data->v);
    }
    free(node->data);
    free(node);
}

// Destroy the trie and free all storage.
void ibt_destroy( Trie trie){
    free_nodes(trie, trie->root);
    free(trie);

}

// insert an entry into the Trie as long as the entry is not already present
void ibt_insert(Trie trie, ikey_t ip, Value v){
    trie->size++;

    if(trie->root == NULL){
        trie->root = malloc(sizeof(struct Node_s));
        if(trie->root == NULL){
            fprintf(stderr, "error: memory allocation failed");
            ibt_destroy(trie);
            exit(EXIT_FAILURE);
        }
        trie->root->left = NULL;
        trie->root->right = NULL;
        trie->root->data = create_entry(ip, v);
        trie->root->height = 1;
        trie->height = 1;
        

        return;
    }


    Node current_node = trie->root;

    for(int i = BITSPERWORD-1; i >= 0; i--){
        char bit = (ip >> i) & 1;
        
        if(current_node->data != NULL){

            if(current_node->data->key == ip){
                (trie->size)--;
                free(v);
                return;
            }

            if( ((current_node->data->key >> i) & 1) == 1){
                current_node->right = malloc(sizeof(struct Node_s));
                if(current_node->right == NULL){
                    fprintf(stderr, "error: memory allocation failed");
                    ibt_destroy(trie);
                    exit(EXIT_FAILURE);
                }

                current_node->right->left = NULL;
                current_node->right->right = NULL;
                current_node->right->data = current_node->data;
                current_node->right->height = current_node->height + 1;
                current_node->data = NULL;
                (trie->node_count)++;
            } else {
                current_node->left = malloc(sizeof(struct Node_s));
                if(current_node->left == NULL){
                    fprintf(stderr, "error: memory allocation failed");
                    ibt_destroy(trie);
                    exit(EXIT_FAILURE);
                }

                current_node->left->left = NULL;
                current_node->left->right = NULL;
                current_node->left->data = current_node->data;
                current_node->left->height = current_node->height + 1;
                current_node->data = NULL;
                (trie->node_count)++;
            }
        }

        if( bit == 1 ){
            if(current_node->right == NULL){
                current_node->right = malloc(sizeof(struct Node_s));
                if(current_node->right == NULL){
                    fprintf(stderr, "error: memory allocation failed");
                    ibt_destroy(trie);
                    exit(EXIT_FAILURE);
                }
                current_node->right->left = NULL;
                current_node->right->right = NULL;
                current_node->right->data = create_entry(ip, v);
                current_node->right->height = current_node->height + 1;
                if(current_node->right->height > trie->height){
                    trie->height = current_node->right->height;
                }
                return;
            }

            current_node = current_node->right;
        } else {
            if(current_node->left == NULL){
                current_node->left = malloc(sizeof(struct Node_s));
                if(current_node->left == NULL){
                    fprintf(stderr, "error: memory allocation failed");
                    ibt_destroy(trie);
                    exit(EXIT_FAILURE);
                }
                current_node->left->left = NULL;
                current_node->left->right = NULL;
                current_node->left->data = create_entry(ip, v);
                current_node->left->height = current_node->height + 1;
                if(current_node->left->height > trie->height){
                    trie->height = current_node->left->height;
                }
                return;
            }

            current_node = current_node->left;
        }
    }
}

// search for the key in the trie by finding the closest entry that matches key in the Trie
Entry ibt_search( Trie trie, ikey_t key){
    bool find_leftmost = false;
    bool find_rightmost = false;

    Node current_node = trie->root;
    for(int i = BITSPERWORD-1; i >= 0; i--){
        char bit = (key >> i) & 1;

        if(current_node->data != NULL){
            return current_node->data;
        }
 
        if(find_leftmost){
            if(current_node->left != NULL){
                current_node = current_node->left;
            } else {
                current_node = current_node->right;
            }
            continue;
        }
        if(find_rightmost){
            if(current_node->right != NULL){
                current_node = current_node->right;
            } else {
                current_node = current_node->left;
            }
            continue;
        }

        if( bit == 1 ){
            if(current_node->right != NULL){
                current_node = current_node->right;
            } else {
                current_node = current_node->left;
                find_rightmost = true;
            }

        } else {
            if(current_node->left != NULL){
                current_node = current_node->left;
            } else {
                current_node = current_node->right;
                find_leftmost = true;
            }

            
        }
    }

    if(current_node->data != NULL){
        return current_node->data;
    }

    return NULL; //If my code works right this should never happen
}

// get the number of leaf nodes in the tree
size_t ibt_size( Trie trie){
    return trie->size;
}

// get the number of internal nodes in the tree
size_t ibt_node_count( Trie trie){
    return trie->node_count;
}

// get the height of the tree
long int ibt_height( Trie trie){
    return trie->height;
}

/*
 * Prints out each node in the trie based on the passed function pointer, or in hexadecimal
 *  if pointer is NULL
 * 
 * @param node              The current node to print
 * @param stream            Where to print the output to
 * @param print_values      A pointer to a function that prints out the values in the node
 */
void show_tree(Node node, FILE * stream, void (*print_values)(Entry entry, FILE * stream) ){
    if(node == NULL){
        return;
    }

    show_tree(node->left, stream, print_values);

    if(node->data != NULL){
        if(print_values != NULL){
            print_values(node->data, stream);
        } else {
            fprintf(stream, "0x%x: %p\n", node->data->key, node->data->v);
        }
    }

    show_tree(node->right, stream, print_values);
}

// Perform an in-order traversal to show each (key, value) in the trie
// Uses Trie's Show_value function to show each leaf node's data,
// and if the function is NULL, output each key and value in hexadecimal
void ibt_show(Trie trie, FILE * stream){
    fprintf(stream, "keys: \n");
    show_tree(trie->root, stream, trie->print_values);
}
