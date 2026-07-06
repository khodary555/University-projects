#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

typedef struct Node
{
	char word[100];
	int height;
	struct Node *left, *right;
} Node;

int height(Node *n)
{
	if (n == NULL)
		return 0;
	return n->height;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

int getBalance(Node *n)
{
	if (n == NULL)
		return 0;
	return height(n->left) - height(n->right);
}

Node *newNode(char *word)
{
	Node *n = (Node *)malloc(sizeof(Node));
	strcpy(n->word, word);
	n->height = 1;
	n->left = n->right = NULL;
	return n;
}

Node *rotateRight(Node *y)
{
	Node *x = y->left;
	Node *T2 = x->right;
	x->right = y;
	y->left = T2;
	y->height = 1 + max(height(y->left), height(y->right));
	x->height = 1 + max(height(x->left), height(x->right));
	return x;
}

Node *rotateLeft(Node *x)
{
	Node *y = x->right;
	Node *T2 = y->left;
	y->left = x;
	x->right = T2;
	x->height = 1 + max(height(x->left), height(x->right));
	y->height = 1 + max(height(y->left), height(y->right));
	return y;
}

Node *insert(Node *node, char *word)
{
	if (node == NULL)
		return newNode(word);

	int cmp = strcasecmp(word, node->word);
	if (cmp < 0)
		node->left = insert(node->left, word);
	else if (cmp > 0)
		node->right = insert(node->right, word);
	else
		return node;

	node->height = 1 + max(height(node->left), height(node->right));

	int balance = getBalance(node);

	// Left Left
	if (balance > 1 && strcasecmp(word, node->left->word) < 0)
		return rotateRight(node);

	// Right Right
	if (balance < -1 && strcasecmp(word, node->right->word) > 0)
		return rotateLeft(node);

	// Left Right
	if (balance > 1 && strcasecmp(word, node->left->word) > 0)
	{
		node->left = rotateLeft(node->left);
		return rotateRight(node);
	}

	// Right Left
	if (balance < -1 && strcasecmp(word, node->right->word) < 0)
	{
		node->right = rotateRight(node->right);
		return rotateLeft(node);
	}

	return node;
}

int treeSize(Node *root)
{
	if (root == NULL)
		return 0;
	return 1 + treeSize(root->left) + treeSize(root->right);
}

int treeHeight(Node *root)
{
	if (root == NULL)
		return 0;
	return root->height;
}

Node *search(Node *root, char *word, Node **last, Node **pred, Node **succ)
{
	Node *curr = root;
	*last = NULL;
	*pred = NULL;
	*succ = NULL;

	while (curr != NULL)
	{
		int cmp = strcasecmp(word, curr->word);
		if (cmp == 0)
			return curr;

		*last = curr;

		if (cmp < 0)
		{
			*succ = curr;
			curr = curr->left;
		}
		else
		{
			*pred = curr;
			curr = curr->right;
		}
	}
	return NULL;
}

Node *inorderPredecessor(Node *root, Node *target)
{
	Node *pred = NULL;
	Node *curr = root;
	while (curr != NULL)
	{
		int cmp = strcasecmp(target->word, curr->word);
		if (cmp > 0)
		{
			pred = curr;
			curr = curr->right;
		}
		else if (cmp < 0)
		{
			curr = curr->left;
		}
		else
		{

			if (curr->left != NULL)
			{
				Node *tmp = curr->left;
				while (tmp->right != NULL)
					tmp = tmp->right;
				pred = tmp;
			}
			break;
		}
	}
	return pred;
}

Node *inorderSuccessor(Node *root, Node *target)
{
	Node *succ = NULL;
	Node *curr = root;
	while (curr != NULL)
	{
		int cmp = strcasecmp(target->word, curr->word);
		if (cmp < 0)
		{
			succ = curr;
			curr = curr->left;
		}
		else if (cmp > 0)
		{
			curr = curr->right;
		}
		else
		{

			if (curr->right != NULL)
			{
				Node *tmp = curr->right;
				while (tmp->left != NULL)
					tmp = tmp->left;
				succ = tmp;
			}
			break;
		}
	}
	return succ;
}

int main()
{
	Node *root = NULL;
	FILE *file = fopen("dictionary.txt", "r");
	if (file == NULL)
	{
		printf("Error: Could not open Dictionary.txt\n");
		return 1;
	}

	char word[100];
	while (fscanf(file, "%s", word) != EOF)
	{
		root = insert(root, word);
	}
	fclose(file);

	printf("Dictionary Loaded Successfully...!\n");
	printf("Size = %d\n", treeSize(root));
	printf("Height = %d\n", treeHeight(root));

	printf("Enter a sentence :\n");
	char sentence[1000];
	fgets(sentence, sizeof(sentence), stdin);
	sentence[strcspn(sentence, "\n")] = 0;

	int hasLetter = 0;
	for (int i = 0; sentence[i] != '\0'; i++)
	{
		if (isalpha(sentence[i]))
		{
			hasLetter = 1;
			break;
		}
	}

	if (strlen(sentence) == 0 || !hasLetter)
	{
		printf("Error: Please enter a valid sentence with at least one word.\n");
		return 1;
	}

	char *token = strtok(sentence, " ");
	while (token != NULL)
	{
		Node *last, *pred, *succ;
		Node *found = search(root, token, &last, &pred, &succ);

		if (found != NULL)
		{
			printf("%s - CORRECT\n", token);
		}
		else
		{
			Node *p = inorderPredecessor(root, last);
			Node *s = inorderSuccessor(root, last);

			printf("%s - Incorrect, Suggestions : ", token);
			printf("%s ", last->word);
			if (p != NULL)
				printf("%s ", p->word);
			if (s != NULL)
				printf("%s", s->word);
			printf("\n");
		}

		token = strtok(NULL, " ");
	}

	return 0;
}