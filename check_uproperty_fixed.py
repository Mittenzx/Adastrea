import os
import re

def check_uproperty_usage():
    '''Check for UObject pointers without UPROPERTY macro'''
    print('=== UPROPERTY Usage Audit ===')

    # Find all .h files
    header_files = []
    for root, dirs, files in os.walk('Source'):
        for file in files:
            if file.endswith('.h'):
                header_files.append(os.path.join(root, file))

    issues_found = []

    for header_file in header_files:
        try:
            with open(header_file, 'r', encoding='utf-8') as f:
                content = f.read()

            # Find UObject pointer declarations
            # Look for patterns like: UClassName* VariableName;
            uobject_patterns = [
                r'\bU[A-Z][a-zA-Z0-9_]*\s*\*\s*[a-zA-Z_][a-zA-Z0-9_]*\s*;',
                r'\bTObjectPtr<\s*U[A-Z][a-zA-Z0-9_]*\s*>\s*[a-zA-Z_][a-zA-Z0-9_]*\s*;',
                r'\bTSubclassOf<\s*U[A-Z][a-zA-Z0-9_]*\s*>\s*[a-zA-Z_][a-zA-Z0-9_]*\s*;'
            ]

            lines = content.split('\n')
            for line_num, line in enumerate(lines, 1):
                # Skip comments and generated code
                if line.strip().startswith('//') or 'GENERATED_' in line:
                    continue

                # Check if line has UPROPERTY (current line or previous line)
                has_uproperty = 'UPROPERTY' in line
                if line_num > 1:
                    has_uproperty = has_uproperty or 'UPROPERTY' in lines[line_num - 2]

                for pattern in uobject_patterns:
                    matches = re.finditer(pattern, line)
                    for match in matches:
                        var_declaration = match.group(0).strip()
                        if not has_uproperty and not line.strip().startswith('*'):  # Skip function parameters
                            issues_found.append({
                                'file': header_file,
                                'line': line_num,
                                'declaration': var_declaration,
                                'context': line.strip()
                            })

        except Exception as e:
            print(f'Error reading {header_file}: {e}')

    if issues_found:
        print(f'Found {len(issues_found)} potential UPROPERTY issues:')
        for issue in issues_found[:10]:  # Show first 10
            print(f'  {issue["file"]}:{issue["line"]} - {issue["declaration"]}')
            print(f'    Context: {issue["context"]}')
        if len(issues_found) > 10:
            print(f'  ... and {len(issues_found) - 10} more issues')
    else:
        print('✅ No UPROPERTY issues found!')

    return len(issues_found)

if __name__ == "__main__":
    check_uproperty_usage()