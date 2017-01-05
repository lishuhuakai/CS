function [all_theta] = oneVsAll(X, y, num_labels, lambda)

%ONEVSALL trains multiple logistic regression classifiers and returns all
%the classifiers in a matrix all_theta, where the i-th row of all_theta 
%corresponds to the classifier for label i
%   [all_theta] = ONEVSALL(X, y, num_labels, lambda) trains num_labels
%   logisitc regression classifiers and returns each of these classifiers
%   in a matrix all_theta, where the i-th row of all_theta corresponds 
%   to the classifier for label i

% 好吧,我们来看一下这个函数是怎么来实现的吧.

% Some useful variables
m = size(X, 1); % 训练集的个数
n = size(X, 2); % 这个应该是特征的个数

% You need to return the following variables correctly 
% num_labels表示label的数目,在这些代码里面,num_labels=10
all_theta = zeros(num_labels, n + 1);

% Add ones to the X data matrix
X = [ones(m, 1) X]; % 这里的话,主要是将第1列全部变为1

% ====================== YOUR CODE HERE ======================
% Instructions: You should complete the following code to train num_labels
%               logistic regression classifiers with regularization
%               parameter lambda. 
%
% Hint: theta(:) will return a column vector.
%
% Hint: You can use y == c to obtain a vector of 1's and 0's that tell use 
%       whether the ground truth is true/false for this class.
%
% Note: For this assignment, we recommend using fmincg to optimize the cost
%       function. It is okay to use a for-loop (for c = 1:num_labels) to
%       loop over the different classes.
%
%       fmincg works similarly to fminunc, but is more efficient when we
%       are dealing with large number of parameters.
%
% Example Code for fmincg:
%
%     % Set Initial theta
%     initial_theta = zeros(n + 1, 1);
%     
%     % Set options for fminunc
%     options = optimset('GradObj', 'on', 'MaxIter', 50);
% 
%     % Run fmincg to obtain the optimal theta
%     % This function will return theta and the cost 
%     [theta] = ...
%         fmincg (@(t)(lrCostFunction(t, X, (y == c), lambda)), ...
%                 initial_theta, options);
%
% 我们到底要干什么,这个一定要清楚, all_theta是一个10*401类型的矩阵
for c = 1:num_labels % 一共要进行10次
    initial_theta = zeros(n + 1, 1); % 初始的theta值,x0等于1
    options = optimset('GradObj', 'on', 'MaxIter', 50);
    % 然后训练出theta的值
    [theta] = fmincg (@(t)(lrCostFunction(t, X, (y == c), lambda)), initial_theta, options);
    % theta是一个401 * 1类型的矩阵,也就是说要训练出401个参数
    all_theta(c, :) = theta';
end
% =========================================================================
end
