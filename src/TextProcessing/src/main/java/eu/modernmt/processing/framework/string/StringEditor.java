package eu.modernmt.processing.framework.string;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by lucamastrostefano on 25/03/16.
 */
public class StringEditor {

    private List<ProcessedString.Operation> changeLog;
    private ProcessedString processedString;
    private int lastEditedIndex;
    private int deltaIndexes;
    private boolean inUse;

    protected StringEditor(ProcessedString processedString) {
        this.processedString = processedString;
    }

    protected void init() {
        this.changeLog = new LinkedList<>();
        this.lastEditedIndex = -1;
        this.deltaIndexes = 0;
        this.inUse = true;
    }

    public void replace(int startIndex, int length, String replace, ProcessedString.TokenType tokenType) {
        if (!this.inUse) {
            throw new RuntimeException("Closed editor");
        }
        if (startIndex > this.lastEditedIndex) {
            ProcessedString.Operation operation = new ProcessedString.Operation();
            operation.startIndex = startIndex + this.deltaIndexes;
            operation.length = length;
            if (replace == null) {
                operation.lengthNewString = length;
            } else {
                operation.newString = replace;
                operation.lengthNewString = replace.length();
            }
            operation.tokenType = tokenType;
            this.changeLog.add(operation);
            this.lastEditedIndex = startIndex + length - 1;
            this.deltaIndexes += (operation.lengthNewString - operation.length);
        } else {
            throw new InvalidOperationException(startIndex, this.lastEditedIndex);
        }
    }

    public void replace(int startIndex, int length, String string) {
        this.replace(startIndex, length, string, null);
    }

    public void delete(int startIndex, int length) {
        this.replace(startIndex, length, "");
    }

    public void insert(int startIndex, String string) {
        this.replace(startIndex, 0, string);
    }

    public void setWord(int startIndex, int length, String replace) {
        replace(startIndex, length, replace, ProcessedString.TokenType.Word);
    }

    public void setWord(int startIndex, int length) {
        replace(startIndex, length, null, ProcessedString.TokenType.Word);
    }

    public void setXMLTag(int startIndex, int length) {
        replace(startIndex, length, " ", ProcessedString.TokenType.XML);
    }

    public ProcessedString commitChanges() {
        this.processedString.applyOperations(this.changeLog);
        this.changeLog = null;
        this.inUse = false;

        return this.processedString;
    }

    public ProcessedString discardChanges() {
        this.changeLog = null;
        this.inUse = false;

        return this.processedString;
    }

    protected boolean isInUse() {
        return inUse;
    }
}

