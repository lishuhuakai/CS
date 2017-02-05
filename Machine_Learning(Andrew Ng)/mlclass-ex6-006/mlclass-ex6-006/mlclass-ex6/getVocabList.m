function vocabList = getVocabList()
%GETVOCABLIST reads the fixed vocabulary list in vocab.txt and returns a
%cell array of the words
%   vocabList = GETVOCABLIST() reads the fixed vocabulary list in vocab.txt 
%   and returns a cell array of the words in vocabList.


%% Read the fixed vocabulary list
fid = fopen('vocab.txt');

% Store all dictionary words in cell array vocab{}
n = 1899;  % 字典里面所拥有的词语的数目

% For ease of implementation, we use a struct to map the strings => integers
% In practice, you'll want to use some form of hashmap
% 为了实现起来更加方便,我们构建一个映射,从strings映射到int类型的数.
vocabList = cell(n, 1); % n代表词语的数目
for i = 1:n
    % Word Index (can ignore since it will be = i)
    fscanf(fid, '%d', 1);
    % Actual Word
    vocabList{i} = fscanf(fid, '%s', 1);
end
fclose(fid);

end
