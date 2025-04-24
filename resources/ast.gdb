break parse
commands
    finish
end
set pagination off
run
set print pretty on

set $current = head
print *$current

define ast_tlc_next
    set $current = $current->next
    print *$current
end

define ast_statements
    set $current = $current->ast_node_tlc_function_declaration->statements
    print *$current
end

define ast_statements_begin
    set $stmt_list = $current->ast_node_stmt_block->statements
    set $stmt_count = $stmt_list.count
    set $stmt_index = 1
    set $statement = $stmt_list.first
    if $statement != 0
        printf "Statement 1/%d:\n", $stmt_count
        print *$statement
    else
        echo No statements in block.\n
    end
end


define ast_statements_next
    if $stmt_index >= $stmt_count
        echo Reached end of statement list.\n
    else
        set $statement = $statement->next
        set $stmt_index = $stmt_index + 1
        if $statement != 0
            printf "Statement %d/%d:\n", $stmt_index, $stmt_count
            print *$statement
        else
            echo Statement pointer is NULL before reaching expected count.\n
        end
    end
end

define ast_tlc_declaration
    set $current = $current->ast_node_tlc_declaration->expr
    print *$current
end

define ast_expr_binary
    set $left = $current->ast_node_expr_binary->left
    set $operation = $current->ast_node_expr_binary.operation
    set $right = $current->ast_node_expr_binary->right

    echo Left
    print *$left
    echo Operation
    print $operation
    echo right
    print *$right
end