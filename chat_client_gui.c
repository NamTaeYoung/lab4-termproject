#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096

// Global variables
GtkWidget *text_view;
GtkTextBuffer *text_buffer;
int client_socket;

// Function to append text to the chat window
void append_text_to_chat(const char *text) {
    if (!text_buffer) {
        g_warning("Text buffer is null. Cannot append text.");
        return;
    }
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gtk_text_buffer_insert(text_buffer, &end, text, -1);
    gtk_text_buffer_insert(text_buffer, &end, "\n", -1); // Add newline
}

// Thread function to receive messages from the server
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            append_text_to_chat("Disconnected from server.");
            close(client_socket);
            exit(0);
        }
        append_text_to_chat(buffer);
    }
    return NULL;
}

// Callback function for the "Send" button
void on_send_button_clicked(GtkWidget *widget, gpointer entry) {
    const char *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(message) > 0) {
        char formatted_message[BUFFER_SIZE];
        snprintf(formatted_message, BUFFER_SIZE, "You: %s", message);
        append_text_to_chat(formatted_message);

        send(client_socket, message, strlen(message), 0);
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // Clear the entry field
    }
}

// Callback function for the "Send File" button
void on_send_file_button_clicked(GtkWidget *widget, gpointer entry) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select a file to send",
                                                    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            append_text_to_chat("Failed to open file.");
            g_free(filename);
            gtk_widget_destroy(dialog);
            return;
        }

        // Notify the server about the file transfer
        char file_transfer_message[BUFFER_SIZE];
        snprintf(file_transfer_message, BUFFER_SIZE, "/file %s", filename);
        send(client_socket, file_transfer_message, strlen(file_transfer_message), 0);

        // Send the file in chunks
        char file_buffer[FILE_BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(file_buffer, 1, FILE_BUFFER_SIZE, file)) > 0) {
            send(client_socket, file_buffer, bytes_read, 0);
        }

        fclose(file);
        append_text_to_chat("File sent successfully.");
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Set up socket connection
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with server IP if needed
    server_addr.sin_port = htons(8080);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // Request username
    char username[BUFFER_SIZE];
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    send(client_socket, username, strlen(username), 0);

    // Create GTK window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create vertical box container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create chat display area
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Create entry and send button
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *entry = gtk_entry_new();
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    GtkWidget *send_file_button = gtk_button_new_with_label("Send File");
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), send_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), send_file_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Connect the "Send" and "Send File" button signals
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), entry);
    g_signal_connect(send_file_button, "clicked", G_CALLBACK(on_send_file_button_clicked), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start receiving messages in a separate thread
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    // Start GTK main loop
    gtk_main();

    // Close the socket when the program exits
    close(client_socket);

    return 0;
}