
#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

#pragma endregion

#pragma region "Compare"

	static int cmp_by_name(t_symbol *a, t_symbol *b) {
		return (ft_strcmp(a->name, b->name));
	}

	static int cmp_by_value(t_symbol *a, t_symbol *b) {
		if (!a->has_value && !b->has_value)	return (ft_strcmp(a->name, b->name));
		if (!a->has_value)					return (-1);
		if (!b->has_value)					return (1);
		if (a->value > b->value)			return (1);

		return (0);
	}

#pragma endregion

#pragma region "Split"

	// Splits the list in half
	static t_symbol *split_half(t_symbol *head) {
		t_symbol *slow = head;
		t_symbol *fast = head->next;

		while (fast && fast->next) {
			slow = slow->next;
			fast = fast->next->next;
		}
		fast = slow->next;
		slow->next = NULL;

		return (fast);
	}

#pragma endregion

#pragma region "Merge"

	// Merges two sorted lists into one sorted list
	static t_symbol *merge(t_symbol *a, t_symbol *b, int by_value) {
		t_symbol dummy;
		t_symbol *tail = &dummy;

		int (*cmp)(t_symbol *, t_symbol *);
		cmp = (by_value) ? cmp_by_value : cmp_by_name;

		dummy.next = NULL;
		while (a && b) {
			if (cmp(a, b) <= 0) {
				tail->next = a;
				a = a->next;
			} else {
				tail->next = b;
				b = b->next;
			}
			tail = tail->next;
		}

		if (a != NULL)	tail->next = a;
		else			tail->next = b;

		return (dummy.next);
	}

#pragma endregion

#pragma region "Sort"

	// Recursively splits and merges the list
	t_symbol *merge_sort(t_symbol *head, int by_value) {
		t_symbol *second_half;

		if (!head || !head->next) return (head);

		second_half = split_half(head);
		head = merge_sort(head, by_value);
		second_half = merge_sort(second_half, by_value);

		return (merge(head, second_half, by_value));
	}

#pragma endregion
