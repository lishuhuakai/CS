function [X_norm, mu, sigma] = featureNormalize(X)
%FEATURENORMALIZE Normalizes the features in X 
%   FEATURENORMALIZE(X) returns a normalized version of X where
%   the mean value of each feature is 0 and the standard deviation
%   is 1. This is often a good preprocessing step to do when
%   working with learning algorithms.

% You need to set these values correctly
X_norm = X; % 在这个例子里面 X 有两列
mu = zeros(1, size(X, 2)); % 在这个例子中mu是一个1*2维的矩阵
sigma = zeros(1, size(X, 2)); % sigma也是一个1*2维的矩阵

% ====================== YOUR CODE HERE ======================
% Instructions: First, for each feature dimension, compute the mean
%               of the feature and subtract it from the dataset,
%               storing the mean value in mu. Next, compute the 
%               standard deviation of each feature and divide
%               each feature by it's standard deviation, storing
%               the standard deviation in sigma. 
%
%               Note that X is a matrix where each column is a 
%               feature and each row is an example. You need 
%               to perform the normalization separately for 
%               each feature. 
%
% Hint: You might find the 'mean' and 'std' functions useful.
%
mu = mean(X); % 求得每一个feature的平均值
X_norm = X_norm - mu; % 然后每一列都减去平均值
sigma = std(X);% 然后要计算标准差
X_norm = X_norm ./ sigma; % 每一个元素除以给feature的标准差 
% ============================================================

end
