#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define T 3 // Minimum degree for B-tree

// Structure for timestamp
struct Timestamp {
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
};

// Structure for transaction node
typedef struct Transaction {
    char transactionID[20];
    char buyerID[20];
    char sellerID[20];
    double energyAmount;
    double pricePerkWh;
    double totalPrice;
    struct Timestamp timestamp;
}Transaction;

// Structure for BTreeNode (Transaction Node)
typedef struct BTreeNode {
    struct Transaction keys[2 * T - 1];
    struct BTreeNode *C[2 * T];
    int n;
    bool leaf;
}BTreeNode;

typedef struct buyers{
    char buyer_id[20];
    double energy_bought;
    struct BTreeNode* transactionsubtree;
}buyers;

typedef struct buyer_btree_node{
    struct buyers keys[2*T-1];
    struct buyer_btree_node* C[2*T];
    int n;
    bool leaf;
}buyer_btree_node;

typedef struct sellers{
    char seller_id[20];
    float rate_below_300;
    float rate_above_300;
    double total_revenue;
    struct BTreeNode* transactionsubtree;
} sellers;

typedef struct seller_btree_node{
    struct sellers keys[2*T-1];
    struct seller_btree_node* C[2*T];
    int n;
    bool leaf;
}seller_btree_node;

struct BuyerSeller {
    char buyerID[20];
    char sellerID[20];
    int no_of_tr;
    double total_energy;
};

// Structure for B-tree node
struct BuyerSeller_btree_node {
    struct BuyerSeller keys[2 * T - 1];
    struct BuyerSeller_btree_node *C[2 * T];
    int n;
    bool leaf;
};


struct BuyerSeller_btree_node* createBuyerSellerTreeNode(bool leaf) {
    struct BuyerSeller_btree_node* node = (struct BuyerSeller_btree_node*)malloc(sizeof(struct BuyerSeller_btree_node));
    node->leaf = leaf;
    node->n = 0;
    for (int i = 0; i < 2 * T; i++)
        node->C[i] = NULL;
    return node;
}

void split_BuyerSeller_Child(struct BuyerSeller_btree_node *parent, int i, struct BuyerSeller_btree_node *y) {
    struct BuyerSeller_btree_node *z = createBuyerSellerTreeNode(y->leaf);
    z->n = T - 1;

    // Copy last (T-1) keys of y to z
    for (int j = 0; j < T - 1; j++)
        z->keys[j] = y->keys[j + T];

    // Copy the child pointers if not leaf
    if (!y->leaf) {
        for (int j = 0; j < T; j++)
            z->C[j] = y->C[j + T];
    }

    y->n = T - 1;

    // Move children of parent to make space
    for (int j = parent->n; j >= i + 1; j--)
        parent->C[j + 1] = parent->C[j];

    parent->C[i + 1] = z;

    // Move keys in parent node
    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];

    parent->keys[i] = y->keys[T - 1];
    parent->n++;
}

int compareBuyerSellerStructs(struct BuyerSeller a, struct BuyerSeller b) {
    if (a.no_of_tr != b.no_of_tr)
        return a.no_of_tr - b.no_of_tr;
    int cmpBuyer = strcmp(a.buyerID, b.buyerID);
    if (cmpBuyer != 0)
        return cmpBuyer;
    return strcmp(a.sellerID, b.sellerID);
}

void insertBuyerSeller(struct BuyerSeller_btree_node **root, struct BuyerSeller k) {
    struct BuyerSeller_btree_node *r = *root;

    if (r->n == 2 * T - 1) {
        struct BuyerSeller_btree_node *s = createBuyerSellerTreeNode(false);
        *root = s;
        s->C[0] = r;
        split_BuyerSeller_Child(s, 0, r);

        int i = compareBuyerSellerStructs(k, s->keys[0]) > 0 ? 1 : 0;
        insertBuyerSeller(&(s->C[i]), k);
    } else {
        int i = r->n - 1;

        if (r->leaf) {
            while (i >= 0 && compareBuyerSellerStructs(k, r->keys[i]) < 0) {
                r->keys[i + 1] = r->keys[i];
                i--;
            }
            r->keys[i + 1] = k;
            r->n++;
        } else {
            while (i >= 0 && compareBuyerSellerStructs(k, r->keys[i]) < 0)
                i--;

            i++;

            if (r->C[i]->n == 2 * T - 1) {
                split_BuyerSeller_Child(r, i, r->C[i]);

                if (compareBuyerSellerStructs(k, r->keys[i]) > 0)
                    i++;
            }
            insertBuyerSeller(&(r->C[i]), k);
        }
    }
}
int calculateTransactionsofBuyerSellerfromTransactions(BTreeNode* transactionRoot, const char* buyerID, const char* sellerID) {
    if (transactionRoot == NULL) return 0;
    int count = 0;
    for (int i = 0; i < transactionRoot->n; i++) {
        if (strcmp(transactionRoot->keys[i].buyerID, buyerID) == 0 && strcmp(transactionRoot->keys[i].sellerID, sellerID) == 0) {
            count++;
        }
    }
    for (int i = 0; i <= transactionRoot->n; i++) {
        count += calculateTransactionsofBuyerSellerfromTransactions(transactionRoot->C[i], buyerID, sellerID);
    }
    return count;
}


void buildBuyerSellerTreeFromTransactions(BTreeNode *transactionRoot, struct BuyerSeller_btree_node **buyerSellerRoot) {
    if (transactionRoot == NULL) return;

    for (int i = 0; i < transactionRoot->n; i++) {
        struct Transaction t = transactionRoot->keys[i];
        // Search if this buyer-seller pair already exists
        struct BuyerSeller_btree_node *current = *buyerSellerRoot;
        bool found = false;

        while (current != NULL) {
            int j = 0;
            while (j < current->n && strcmp(t.buyerID, current->keys[j].buyerID) > 0) j++;

            while (j < current->n && strcmp(t.buyerID, current->keys[j].buyerID) == 0 && strcmp(t.sellerID, current->keys[j].sellerID) > 0)
                j++;

            if (j < current->n && strcmp(t.buyerID, current->keys[j].buyerID) == 0 && strcmp(t.sellerID, current->keys[j].sellerID) == 0) {
                current->keys[j].total_energy += t.energyAmount;
                found = true;
                break;
            }

            if (current->leaf) break;
            current = current->C[j];
        }

        // If not found, insert new BuyerSeller node
        if (!found) {
            struct BuyerSeller bs;
            strcpy(bs.buyerID, t.buyerID);
            strcpy(bs.sellerID, t.sellerID);
            bs.total_energy = t.energyAmount;
            bs.no_of_tr = calculateTransactionsofBuyerSellerfromTransactions(transactionRoot, t.buyerID, t.sellerID);
            insertBuyerSeller(buyerSellerRoot, bs);
        }
    }

    // Recur on child subtrees
    if (!transactionRoot->leaf) {
        for (int i = 0; i <= transactionRoot->n; i++) {
            buildBuyerSellerTreeFromTransactions(transactionRoot->C[i], buyerSellerRoot);
        }
    }
} 



// Create a new transaction
struct Transaction createTransaction(const char *transactionID, const char *buyerID, const char *sellerID, double energyAmount, double pricePerkWh, struct Timestamp timestamp) {
    struct Transaction t;
    strcpy(t.transactionID, transactionID);
    strcpy(t.buyerID, buyerID);
    strcpy(t.sellerID, sellerID);
    t.energyAmount = energyAmount;
    t.pricePerkWh = pricePerkWh;
    t.totalPrice = energyAmount * pricePerkWh;
    t.timestamp = timestamp;
    return t;
}

// Create a new BTreeNode
struct BTreeNode *createBTreeNode(bool leaf) {
    struct BTreeNode *node = (struct BTreeNode *)malloc(sizeof(struct BTreeNode));
    node->leaf = leaf;
    node->n = 0;
    for (int i = 0; i < 2 * T; i++)
        node->C[i] = NULL;
    return node;
}

// Insert transaction into B-tree
void insertTransaction(struct BTreeNode **root, struct Transaction k);
void splitChild(struct BTreeNode *parent, int i, struct BTreeNode *child);

// Utility function to traverse the B-tree
void traverseTransactions(struct BTreeNode *root) {
    if (root == NULL) return;
    int i;
    for (i = 0; i < root->n; i++) {
        if (!root->leaf)
            traverseTransactions(root->C[i]);
        printf("Transaction ID: %s, Buyer ID: %s, Seller ID: %s, Energy: %.2lf kWh, Price: $%.2lf, Timestamp: %02d-%02d-%04d %02d:%02d:%02d\n",
               root->keys[i].transactionID, root->keys[i].buyerID, root->keys[i].sellerID, root->keys[i].energyAmount, root->keys[i].totalPrice,
               root->keys[i].timestamp.day, root->keys[i].timestamp.month, root->keys[i].timestamp.year,
               root->keys[i].timestamp.hour, root->keys[i].timestamp.minute, root->keys[i].timestamp.second);
    }
    if (!root->leaf)
        traverseTransactions(root->C[i]);
}

// Insert a new key in the B-tree
void insertTransaction(struct BTreeNode **root, struct Transaction k) {
    struct BTreeNode *r = *root;
    if (r->n == 2 * T - 1) {
        struct BTreeNode *s = createBTreeNode(false);
        *root = s;
        s->C[0] = r;
        splitChild(s, 0, r);
        int i = (strcmp(s->keys[0].transactionID, k.transactionID) < 0) ? 1 : 0;
        insertTransaction(&(s->C[i]), k);
    } else {
        int i = r->n - 1;
        if (r->leaf) {
            while (i >= 0 && strcmp(r->keys[i].transactionID, k.transactionID) > 0) {
                r->keys[i + 1] = r->keys[i];
                i--;
            }
            r->keys[i + 1] = k;
            r->n++;
        } else {
            while (i >= 0 && strcmp(r->keys[i].transactionID, k.transactionID) > 0)
                i--;
            i++;
            if (r->C[i]->n == 2 * T - 1) {
                splitChild(r, i, r->C[i]);
                if (strcmp(r->keys[i].transactionID, k.transactionID) < 0)
                    i++;
            }
            insertTransaction(&(r->C[i]), k);
        }
    }
}

// Split a full child
void splitChild(struct BTreeNode *parent, int i, struct BTreeNode *child) {
    struct BTreeNode *z = createBTreeNode(child->leaf);
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++)
        z->keys[j] = child->keys[j + T];
    if (!child->leaf)
        for (int j = 0; j < T; j++)
            z->C[j] = child->C[j + T];
    child->n = T - 1;
    for (int j = parent->n; j >= i + 1; j--)
        parent->C[j + 1] = parent->C[j];
    parent->C[i + 1] = z;
    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = child->keys[T - 1];
    parent->n++;
}

sellers createSeller(const char *sellerID, float rateBelow300, float rateAbove300, double totalRevenue) {
    sellers s;
    strcpy(s.seller_id, sellerID);
    s.rate_below_300 = rateBelow300;
    s.rate_above_300 = rateAbove300;
    s.total_revenue = totalRevenue;
    s.transactionsubtree = NULL; 
    return s;
}

struct seller_btree_node *createSellerTreeNode(bool leaf) {
    struct seller_btree_node *node = (struct seller_btree_node *)malloc(sizeof(struct seller_btree_node));
    node->leaf = leaf;
    node->n = 0;
    for (int i = 0; i < 2 * T; i++)
        node->C[i] = NULL;
    return node;
}
void split_Seller_Child(struct seller_btree_node *parent, int i, struct seller_btree_node *child) {
    struct seller_btree_node *z = createSellerTreeNode(child->leaf);
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++)
        z->keys[j] = child->keys[j + T];
    if (!child->leaf)
        for (int j = 0; j < T; j++)
            z->C[j] = child->C[j + T];
    child->n = T - 1;
    for (int j = parent->n; j >= i + 1; j--)
        parent->C[j + 1] = parent->C[j];
    parent->C[i + 1] = z;
    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = child->keys[T - 1];
    parent->n++;
}

void insertSeller(struct seller_btree_node **root, struct sellers k) {
    struct seller_btree_node *r = *root;
    if (r->n == 2 * T - 1) {
        struct seller_btree_node *s = createSellerTreeNode(false);
        *root = s;
        s->C[0] = r;
        split_Seller_Child(s, 0, r);
        int i = (strcmp(s->keys[0].seller_id, k.seller_id) < 0) ? 1 : 0;
        insertSeller(&(s->C[i]), k);
    } else {
        int i = r->n - 1;
        if (r->leaf) {
            while (i >= 0 && strcmp(r->keys[i].seller_id, k.seller_id) > 0) {
                r->keys[i + 1] = r->keys[i];
                i--;
            }
            r->keys[i + 1] = k;
            r->n++;
        } else {
            while (i >= 0 && strcmp(r->keys[i].seller_id, k.seller_id) > 0)
                i--;
            i++;
            if (r->C[i]->n == 2 * T - 1) {
                split_Seller_Child(r, i, r->C[i]);
                if (strcmp(r->keys[i].seller_id, k.seller_id) < 0)
                    i++;
            }
            insertSeller(&(r->C[i]), k);
        }
    }
}

buyers createBuyer(const char *buyerID, float energyBought) {
    buyers b;
    strcpy(b.buyer_id, buyerID);
    b.energy_bought = energyBought;
    b.transactionsubtree= NULL;
    return b;
}

struct buyer_btree_node *createBuyerTreeNode(bool leaf) {
    struct buyer_btree_node *node = (struct buyer_btree_node *)malloc(sizeof(struct buyer_btree_node));
    node->leaf = leaf;
    node->n = 0;
    for (int i = 0; i < 2 * T; i++)
        node->C[i] = NULL;
    return node;
}

void split_Buyer_Child(struct buyer_btree_node *parent, int i, struct buyer_btree_node *child) {
    struct buyer_btree_node *z = createBuyerTreeNode(child->leaf);
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++)
        z->keys[j] = child->keys[j + T];
    if (!child->leaf)
        for (int j = 0; j < T; j++)
            z->C[j] = child->C[j + T];
    child->n = T - 1;
    for (int j = parent->n; j >= i + 1; j--)
        parent->C[j + 1] = parent->C[j];
    parent->C[i + 1] = z;
    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = child->keys[T - 1];
    parent->n++;
}

void insertBuyer(struct buyer_btree_node **root, struct buyers k) {
    struct buyer_btree_node *r = *root;

    if (r->n == 2 * T - 1) {
        struct buyer_btree_node *s = createBuyerTreeNode(false);
        *root = s;
        s->C[0] = r;
        split_Buyer_Child(s, 0, r);

        int i = (s->keys[0].energy_bought < k.energy_bought) ? 1 : 0;
        insertBuyer(&(s->C[i]), k);
    } else {
        int i = r->n - 1;

        if (r->leaf) {
            // Move keys to make room for the new one
            while (i >= 0 && r->keys[i].energy_bought > k.energy_bought) {
                r->keys[i + 1] = r->keys[i];
                i--;
            }
            r->keys[i + 1] = k;
            r->n++;
        } else {
            while (i >= 0 && r->keys[i].energy_bought > k.energy_bought)
                i--;
            i++;
            if (r->C[i]->n == 2 * T - 1) {
                split_Buyer_Child(r, i, r->C[i]);
                if (r->keys[i].energy_bought < k.energy_bought)
                    i++;
            }
            insertBuyer(&(r->C[i]), k);
        }
    }
}

double calculate_total_energy_bought(struct BTreeNode *root, const char *buyerID) {
    if (root == NULL) return 0.0;
    double totalEnergy = 0.0;
    for (int i = 0; i < root->n; i++) {
        if (strcmp(root->keys[i].buyerID, buyerID) == 0) {
            totalEnergy += root->keys[i].energyAmount;
        }
    }
    for (int i = 0; i <= root->n; i++) {
        totalEnergy += calculate_total_energy_bought(root->C[i], buyerID);
    }
    return totalEnergy;
}

void buildBuyerTreeFromTransactions(struct BTreeNode *transactionRoot, struct buyer_btree_node **buyerRoot) {
    if (transactionRoot == NULL) return;

    for (int i = 0; i < transactionRoot->n; i++) {
        struct Transaction t = transactionRoot->keys[i];
        // Search if this buyer already exists
        struct buyer_btree_node *current = *buyerRoot;
        bool found = false;

        while (current != NULL&&!found) {
            int j = 0;
            while (j < current->n && strcmp(t.buyerID, current->keys[j].buyer_id) > 0) j++;

            if (j < current->n && strcmp(t.buyerID, current->keys[j].buyer_id) == 0) {
                found = true;
            }

            if (current->leaf) break;
            current = current->C[j];
        }

        // If not found, insert new Buyer node
        if (!found) {
            struct buyers b = createBuyer(t.buyerID, calculate_total_energy_bought(transactionRoot,t.buyerID));
            insertBuyer(buyerRoot, b);
        }
    }

    // Recur on child subtrees
    if (!transactionRoot->leaf) {
        for (int i = 0; i <= transactionRoot->n; i++) {
            buildBuyerTreeFromTransactions(transactionRoot->C[i], buyerRoot);
        }
    }
}


// Function to calculate total revenue for a seller
double calculate_total_revenue(struct BTreeNode *root, const char *sellerID) {
    if (root == NULL) return 0.0;
    double totalRevenue = 0.0;
    for (int i = 0; i < root->n; i++) {
        if (strcmp(root->keys[i].sellerID, sellerID) == 0) {
            totalRevenue += root->keys[i].totalPrice;
        }
    }
    for (int i = 0; i <= root->n; i++) {
        totalRevenue += calculate_total_revenue(root->C[i], sellerID);
    }
    return totalRevenue;
}

// Function to store seller revenue in each seller node
void store_seller_revenue(struct seller_btree_node *root, struct BTreeNode *transactionRoot) {
    if (root == NULL) return;

    for (int i = 0; i < root->n; i++) {
        root->keys[i].total_revenue = calculate_total_revenue(transactionRoot, root->keys[i].seller_id);
    }

    for (int i = 0; i <= root->n; i++) {
        store_seller_revenue(root->C[i], transactionRoot);
    }
}



void store_buyer_energy(struct buyer_btree_node *root, struct BTreeNode *transactionRoot) {
    if (root == NULL) return;

    for (int i = 0; i < root->n; i++) {
        root->keys[i].energy_bought = calculate_total_energy_bought(transactionRoot, root->keys[i].buyer_id);
    }

    for (int i = 0; i <= root->n; i++) {
        store_buyer_energy(root->C[i], transactionRoot);
    }
}

Transaction *searchTransaction(struct BTreeNode *root, const char *transactionID) {
    if (root == NULL) return NULL;
    int i = 0;
    while (i < root->n && strcmp(transactionID, root->keys[i].transactionID) > 0)
        i++;
    if (i < root->n && strcmp(transactionID, root->keys[i].transactionID) == 0)
        return &root->keys[i];
    if (root->leaf)
        return NULL;
    return searchTransaction(root->C[i], transactionID);
}

buyers *searchBuyer(struct buyer_btree_node *root, const char *buyerID) {
    if (root == NULL) return NULL;
    int i = 0;
    while (i < root->n && strcmp(buyerID, root->keys[i].buyer_id) > 0)
        i++;
    if (i < root->n && strcmp(buyerID, root->keys[i].buyer_id) == 0)
        return &root->keys[i];
    if (root->leaf)
        return NULL;
    return searchBuyer(root->C[i], buyerID);
}

sellers *searchSeller(struct seller_btree_node *root, const char *sellerID) {
    if (root == NULL) return NULL;
    int i = 0;
    while (i < root->n && strcmp(sellerID, root->keys[i].seller_id) > 0)
        i++;
    if (i < root->n && strcmp(sellerID, root->keys[i].seller_id) == 0)
        return &root->keys[i];
    if (root->leaf)
        return NULL;
    return searchSeller(root->C[i], sellerID);
}

bool isEarlier(struct Timestamp t1, struct Timestamp t2) {
    bool result = false;  // Initialize with default as false
    
    if (t1.year < t2.year) {
        result = true;
    } else if (t1.year == t2.year) {
        if (t1.month < t2.month) {
            result = true;
        } else if (t1.month == t2.month) {
            if (t1.day < t2.day) {
                result = true;
            } else if (t1.day == t2.day) {
                if (t1.hour < t2.hour) {
                    result = true;
                } else if (t1.hour == t2.hour) {
                    if (t1.minute < t2.minute) {
                        result = true;
                    } else if (t1.minute == t2.minute) {
                        if (t1.second < t2.second) {
                            result = true;
                        }
                    }
                }
            }
        }
    }
    return result;
}

// Check if t1 is within the range of t1 and t2 (refactored with a single return statement)
bool isInRange(struct Timestamp t, struct Timestamp start, struct Timestamp end) {
    bool result = false;
    
    // Check if t is not earlier than start and is earlier than end
    if (!isEarlier(t, start)) {
        if (isEarlier(t, end)) {
            result = true;
        }
    }
    return result;
}

void traverseTransactionsInRange(struct BTreeNode *root, struct Timestamp start, struct Timestamp end) {
    if (root == NULL) return;
    
    for (int i = 0; i < root->n; i++) {
        if (isInRange(root->keys[i].timestamp, start, end)) {
            printf("Transaction ID: %s, Buyer ID: %s, Seller ID: %s, Energy: %.2lf kWh, Price: $%.2lf, Timestamp: %02d-%02d-%04d %02d:%02d:%02d\n",
                root->keys[i].transactionID, root->keys[i].buyerID, root->keys[i].sellerID, 
                root->keys[i].energyAmount, root->keys[i].totalPrice,
                root->keys[i].timestamp.day, root->keys[i].timestamp.month, root->keys[i].timestamp.year,
                root->keys[i].timestamp.hour, root->keys[i].timestamp.minute, root->keys[i].timestamp.second);
        }
    }
    
    // If the current node is not a leaf, recurse on child nodes
    if (!root->leaf) {
        for (int i = 0; i <= root->n; i++) {
            traverseTransactionsInRange(root->C[i], start, end);
        }
    }
}

//function to create a subtree of transactions for a every buyer
struct BTreeNode* createTransactionSubtreeForBuyer(struct BTreeNode *root, const char *buyerID, struct BTreeNode **subtree) {
    if (root == NULL) return NULL;
    
    for (int i = 0; i < root->n; i++) {
        if (strcmp(root->keys[i].buyerID, buyerID) == 0) {
            struct Transaction t = root->keys[i];
            insertTransaction(subtree, t);
        }
    }
    
    // Recur on child nodes
    if (!root->leaf) {
        for (int i = 0; i <= root->n; i++) {
            createTransactionSubtreeForBuyer(root->C[i], buyerID, subtree);
        }
    }
    return *subtree;
}

void createTransactionSubtreesForAllBuyers(struct BTreeNode *transactionRoot, struct buyer_btree_node *buyerRoot) {
    if (buyerRoot == NULL) return;

    for (int i = 0; i < buyerRoot->n; i++) {
        buyerRoot->keys[i].transactionsubtree = createBTreeNode(true);
        createTransactionSubtreeForBuyer(transactionRoot, buyerRoot->keys[i].buyer_id, &(buyerRoot->keys[i].transactionsubtree));
    }

    for (int i = 0; i <= buyerRoot->n; i++) {
        createTransactionSubtreesForAllBuyers(transactionRoot, buyerRoot->C[i]);
    }
}

void rangeTraversal_transactions(BTreeNode* node, double low, double high) {
    if (node == NULL) return;

    int i;
    for (i = 0; i < node->n; i++) {
        // Traverse the left child first
        if (!node->leaf)
            rangeTraversal_transactions(node->C[i], low, high);

        // Check if the current transaction falls within the energy range
        double energy = node->keys[i].energyAmount;
        if (energy >= low && energy <= high) {
            printf("Transaction ID: %s | Buyer ID: %s | Seller ID: %s | Energy: %.2f | Price/kWh: %.2f | Total: %.2f | Timestamp: %02d-%02d-%04d %02d:%02d:%02d\n",
                   node->keys[i].transactionID,
                   node->keys[i].buyerID,
                   node->keys[i].sellerID,
                   node->keys[i].energyAmount,
                   node->keys[i].pricePerkWh,
                   node->keys[i].totalPrice,
                   node->keys[i].timestamp.day,
                   node->keys[i].timestamp.month,
                   node->keys[i].timestamp.year,
                   node->keys[i].timestamp.hour,
                   node->keys[i].timestamp.minute,
                   node->keys[i].timestamp.second);
        }
    }

    // Traverse the rightmost child
    if (!node->leaf)
        rangeTraversal_transactions(node->C[i],low,high);
}

//function to create a subtree of transactions for a specific seller
BTreeNode* createTransactionSubtreeForSeller(struct BTreeNode *root, const char *sellerID, struct BTreeNode **subtree) {
    if (root == NULL) return NULL;
    
    for (int i = 0; i < root->n; i++) {
        if (strcmp(root->keys[i].sellerID, sellerID) == 0) {
            struct Transaction t = root->keys[i];
            insertTransaction(subtree, t);
        }
    }
    
    // Recur on child nodes
    if (!root->leaf) {
        for (int i = 0; i <= root->n; i++) {
            createTransactionSubtreeForSeller(root->C[i], sellerID, subtree);
        }
    }
    return *subtree;
}
void createTransactionSubtreesForAllSellers(struct BTreeNode *transactionRoot, struct seller_btree_node *sellerRoot) {
    if (sellerRoot == NULL) return;

    for (int i = 0; i < sellerRoot->n; i++) {
        sellerRoot->keys[i].transactionsubtree = createBTreeNode(true);
        createTransactionSubtreeForSeller(transactionRoot, sellerRoot->keys[i].seller_id, &(sellerRoot->keys[i].transactionsubtree));
    }

    for (int i = 0; i <= sellerRoot->n; i++) {
        createTransactionSubtreesForAllSellers(transactionRoot, sellerRoot->C[i]);
    }
}
// Main function
void loadTransactionsFromFile(const char *filename, struct BTreeNode **transactionRoot) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening transaction file");
        return;
    }

    char transactionID[20], buyerID[20], sellerID[20];
    double energyAmount, pricePerkWh;
    int day, month, year, hour, minute, second;

    while (fscanf(file, "%s %s %s %lf %lf %d-%d-%d %d:%d:%d", 
                  transactionID, buyerID, sellerID, &energyAmount, &pricePerkWh, 
                  &day, &month, &year, &hour, &minute, &second) == 11) {
        struct Timestamp timestamp = {day, month, year, hour, minute, second};
        struct Transaction t = createTransaction(transactionID, buyerID, sellerID, energyAmount, pricePerkWh, timestamp);
        insertTransaction(transactionRoot, t);
    }

    fclose(file);
}

void loadSellersFromFile(const char *filename, struct seller_btree_node **sellerRoot) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening seller file");
        return;
    }

    char sellerID[20];
    float rateBelow300, rateAbove300;
    double totalRevenue;

    while (fscanf(file, "%s %f %f %lf", sellerID, &rateBelow300, &rateAbove300, &totalRevenue) == 4) {
        struct sellers s = createSeller(sellerID, rateBelow300, rateAbove300, totalRevenue);
        insertSeller(sellerRoot, s);
    }

    fclose(file);
}
void saveTransactionsToFile(FILE* fp, struct BTreeNode *transactionRoot) {
    if (transactionRoot == NULL) return;
        BTreeNode* node=transactionRoot;
        for (int i = 0; i < transactionRoot->n; i++) {
            fprintf(fp, "%s %s %s %.2lf %.2lf %02d-%02d-%04d %02d:%02d:%02d\n",
                    node->keys[i].transactionID, node->keys[i].buyerID, node->keys[i].sellerID,
                    node->keys[i].energyAmount, node->keys[i].pricePerkWh,
                    node->keys[i].timestamp.day, node->keys[i].timestamp.month, node->keys[i].timestamp.year,
                    node->keys[i].timestamp.hour, node->keys[i].timestamp.minute, node->keys[i].timestamp.second);
        }
        if (!node->leaf) {
            for (int i = 0; i <= node->n; i++) {
                saveTransactionsToFile(fp,node->C[i]);
            }
        }
}

int main() {
    int choice;
    double minEnergy, maxEnergy;
    char sellerID[20];
    char buyerID[20];
    struct Timestamp start, end;
    struct BTreeNode* transactionRoot=NULL;


    while (1) {
        printf("\n----- ENERGY TRANSACTION SYSTEM -----\n");
        printf("1. Add New Transactions\n");
        printf("2. Display All Transactions\n");
        printf("3. Create a set of Transactions for Every Seller\n");
        printf("4. Create a set of Transactions for Every Buyer\n");
        printf("5. Find all transactions in a Given Time Period\n");
        printf("6. Calculate Total Revenue by Seller\n");
        printf("7. Display transactions with Energy Amounts in Range\n");
        printf("8. Sort Buyers Based on Energy Bought\n");
        printf("9. Sort Seller/Buyer Pairs by Number of Transactions\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:

                insertTransaction(transactionRoot,); // defined in trees.c
                break;
            case 2:
                traverseTransactionBTree(transactionRoot);
                break;
            case 3:
                generateSellerTransactionTrees(transactionRoot);
                break;
            case 4:
                generateBuyerTransactionTrees(transactionRoot);
                break;
            case 5:
                printf("Enter start time (DD MM YYYY HH MM SS): ");
                scanf("%d %d %d %d %d %d", &start.day, &start.month, &start.year, &start.hour, &start.minute, &start.second);
                printf("Enter end time (DD MM YYYY HH MM SS): ");
                scanf("%d %d %d %d %d %d", &end.day, &end.month, &end.year, &end.hour, &end.minute, &end.second);
                findTransactionsInTimePeriod(transactionRoot, start, end);
                break;
            case 6:
                printf("Enter Seller ID: ");
                scanf("%s", sellerID);
                calculateRevenueBySeller(transactionRoot, sellerID);
                break;
            case 7:
                printf("Enter Min Energy Amount: ");
                scanf("%lf", &minEnergy);
                printf("Enter Max Energy Amount: ");
                scanf("%lf", &maxEnergy);
                printTransactionsInEnergyRange(transactionRoot, minEnergy, maxEnergy);
                break;
            case 8:
                sortBuyersByEnergyBought(buyerRoot);
                break;
            case 9:
                sortBuyerSellerPairs(buyerSellerRoot);
                break;
            case 0:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}