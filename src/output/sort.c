#pragma region "Includes"

	#include "nm.h"
	#include "utils.h"

#pragma endregion

#pragma region "Compare"

	// Compara dos símbolos por su nombre
	static int cmp_by_name(t_sym *a, t_sym *b, int reverse) {
		int result = ft_strcmp(a->name, b->name);
		return (reverse ? -result : result);
	}

	// Compara dos símbolos por su valor
	// Los símbolos no definidos van antes que los definidos y se ordenan por nombre
	static int cmp_by_value(t_sym *a, t_sym *b, int reverse) {
		if (is_undefined(a) && is_undefined(b))		return (reverse ? -ft_strcmp(a->name, b->name) : ft_strcmp(a->name, b->name));
		if (is_undefined(a))						return (reverse ? 1 : -1);
		if (is_undefined(b))						return (reverse ? -1 : 1);
		if (a->value > b->value)					return (reverse ? -1 : 1);
		if (a->value < b->value)					return (reverse ? 1 : -1);

		return (reverse ? -ft_strcmp(a->name, b->name) : ft_strcmp(a->name, b->name));
	}

#pragma endregion

#pragma region "Merge"

	// Une dos listas ordenadas en una sola lista
	static t_sym *merge(t_sym *a, t_sym *b, int by_value, int reverse) {
		t_sym	dummy;
		t_sym	*tail = &dummy;
		int		(*cmp)(t_sym *, t_sym *, int);

		cmp = (by_value) ? cmp_by_value : cmp_by_name;

		dummy.next = NULL;
		while (a && b) {
			if (cmp(a, b, reverse) <= 0) {
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

#pragma region "Split"

	// Divide la lista por la mitad
	static t_sym *split_half(t_sym *head) {
		t_sym *slow = head;
		t_sym *fast = head->next;

		while (fast && fast->next) {
			slow = slow->next;
			fast = fast->next->next;
		}
		fast = slow->next;
		slow->next = NULL;

		return (fast);
	}

#pragma endregion

#pragma region "Sort"

	// Divide, ordena y fusiona la lista recursivamente
	t_sym *sort(t_sym *head, int by_value, int reverse) {
		if (!head || !head->next) return (head);

		t_sym *second_half;
		second_half	= split_half(head);
		head		= sort(head, by_value, reverse);
		second_half	= sort(second_half, by_value, reverse);

		return (merge(head, second_half, by_value, reverse));
	}

#pragma endregion
