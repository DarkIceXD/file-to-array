#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

const int add_basic_mime_type = 1;
struct file
{
    char *file_name;
    unsigned char *content;
    long content_length;
    struct file *next;
};

struct file_head
{
    struct file *head;
    struct file *tail;
};

struct file *init_file(const char *file_name, unsigned char *content, long content_length)
{
    struct file *f = malloc(sizeof(struct file));
    f->file_name = malloc(strlen(file_name) + 1);
    strcpy(f->file_name, file_name);
    f->content = content;
    f->content_length = content_length;
    f->next = NULL;
    return f;
}

void free_file(struct file *f)
{
    free(f->file_name);
    free(f->content);
    free(f);
}

void list_files_recursively(const char *base_path, struct file_head *head)
{
    struct dirent *dp;
    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0)
            continue;

        if (strcmp(dp->d_name, "..") == 0)
            continue;

        const int len = snprintf(NULL, 0, "%s/%s", base_path, dp->d_name) + 1;
        char *path = malloc(len);
        snprintf(path, len, "%s/%s", base_path, dp->d_name);
        if (dp->d_type == DT_DIR)
        {
            list_files_recursively(path, head);
        }
        else
        {
            FILE *f = fopen(path, "rb");
            if (f)
            {
                fseek(f, 0, SEEK_END);
                const long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);
                unsigned char *string = malloc(fsize + 1);
                fread(string, fsize, 1, f);
                fclose(f);
                string[fsize] = 0;
                if (head->head == NULL)
                {
                    head->head = init_file(path, string, fsize);
                    head->tail = head->head;
                }
                else
                {
                    head->tail->next = init_file(path, string, fsize);
                    head->tail = head->tail->next;
                }
            }
        }
        free(path);
    }
    closedir(dir);
}

static const char *extension_to_type(const char *extension)
{
    struct extension
    {
        const char *file_extension;
        const char *type;
    };
    static const struct extension e[] = {
        {"txt", "text/plain"},
        {"htm", "text/html"},
        {"html", "text/html"},
        {"js", "text/javascript"},
        {"css", "text/css"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"svg", "image/svg+xml"},
    };
    for (int i = 0; i < sizeof(e) / sizeof(e[0]); i++)
    {
        if (strncmp(extension, e[i].file_extension, strlen(e[i].file_extension)))
            continue;

        return e[i].type;
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
        return 0;

    struct file_head head;
    head.head = NULL;
    head.tail = NULL;
    list_files_recursively(argv[1], &head);
    FILE *file = fopen("data.h", "w");
    if (!file)
        return 0;
    const int base_len = strlen(argv[1]);
    fputs("struct entry\n{\n\tconst char *file_name;\n\t", file);
    if (add_basic_mime_type)
        fputs("const char *mime_type;\n\t", file);
    fputs("const int data_size;\n\tconst char *data;\n};\n\nstatic const struct entry entries[] = {", file);
    struct file *elem = head.head;
    char temp[64];
    while (elem)
    {
        fputs("\n\t{\"", file);
        fputs(base_len + elem->file_name, file);
        fputs("\", ", file);
        if (add_basic_mime_type)
        {
            const char *mime_type = extension_to_type(strrchr(base_len + elem->file_name, '.') + 1);
            if (mime_type)
            {
                fputs("\"", file);
                fputs(mime_type, file);
                fputs("\", ", file);
            }
            else
            {
                fputs("0, ", file);
            }
        }
        snprintf(temp, 64, "%ld", elem->content_length);
        fputs(temp, file);
        fputs(", \"", file);
        for (int i = 0; i < elem->content_length; i++)
        {
            snprintf(temp, 64, "\\x%02x", elem->content[i]);
            fputs(temp, file);
        }
        fputs("\"},", file);
        head.head = elem->next;
        free_file(elem);
        elem = head.head;
    }
    fputs("\n};", file);
    fclose(file);
    return 0;
}
