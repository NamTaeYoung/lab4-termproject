#include <gtk/gtk.h>

// Function to handle button clicks
void on_calculate(GtkWidget *widget, gpointer data) {
    const char *button_label = gtk_button_get_label(GTK_BUTTON(widget));
    GtkEntry **entries = (GtkEntry **)data;

    const char *num1_text = gtk_entry_get_text(entries[0]);
    const char *num2_text = gtk_entry_get_text(entries[1]);

    double num1 = atof(num1_text);
    double num2 = atof(num2_text);
    double result = 0;

    if (strcmp(button_label, "+") == 0) {
        result = num1 + num2;
    } else if (strcmp(button_label, "-") == 0) {
        result = num1 - num2;
    } else if (strcmp(button_label, "×") == 0) {
        result = num1 * num2;
    } else if (strcmp(button_label, "÷") == 0) {
        if (num2 != 0) {
            result = num1 / num2;
        } else {
            gtk_entry_set_text(entries[2], "Error: Division by zero");
            return;
        }
    }

    char result_text[32];
    snprintf(result_text, sizeof(result_text), "%.2f", result);
    gtk_entry_set_text(entries[2], result_text);
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *grid, *entry1, *entry2, *result_entry, *add_button, *sub_button, *mul_button, *div_button;

    gtk_init(&argc, &argv);

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simple Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a grid for layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create entries for input and result
    entry1 = gtk_entry_new();
    entry2 = gtk_entry_new();
    result_entry = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(result_entry), FALSE);

    // Create buttons
    add_button = gtk_button_new_with_label("+");
    sub_button = gtk_button_new_with_label("-");
    mul_button = gtk_button_new_with_label("×");
    div_button = gtk_button_new_with_label("÷");

    // Add entries and buttons to the grid
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Number 1:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 0, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Number 2:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 1, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Result:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), result_entry, 1, 2, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), add_button, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sub_button, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), mul_button, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), div_button, 3, 3, 1, 1);

    // Set up button click handlers
    GtkEntry *entries[] = {GTK_ENTRY(entry1), GTK_ENTRY(entry2), GTK_ENTRY(result_entry)};
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_calculate), entries);
    g_signal_connect(sub_button, "clicked", G_CALLBACK(on_calculate), entries);
    g_signal_connect(mul_button, "clicked", G_CALLBACK(on_calculate), entries);
    g_signal_connect(div_button, "clicked", G_CALLBACK(on_calculate), entries);

    // Show all widgets
    gtk_widget_show_all(window);

    // Run the GTK main loop
    gtk_main();

    return 0;
}