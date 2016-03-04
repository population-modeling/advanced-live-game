#ifndef FIELD_H

#define FIELD_H

struct Field {
    int n, m;

    char **bact1;
    int need_env;
    int need_subst2;
    int prod_subst1;
    int reprod_subst1;
    int max_hunger;
    int death_subst1;
    int **hunger;

    char **bact2;
    int need_subst1;
    int prod_subst2;
    int reprod_subst2;

    int **env;
    int **subst1;
    int **subst2;
};

int print_field(struct Field *field);

int get_empty_field(struct Field *field, int n, int m);

int get_random_field(struct Field *field, int n, int m);

int get_field_from_file(struct Field *field);

void free_field(struct Field *field);

void copy_field(struct Field *to, struct Field *from);

void send_field(struct Field *field, int first_line, int last_line, int to);

void recv_field(struct Field *field, int first_line, int last_line, int from);

void copy_info(struct Field *to, struct Field *from);

#endif
