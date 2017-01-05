function plotData(X, y)
%PLOTDATA Plots the data points X and y into a new figure 
%   PLOTDATA(x,y) plots the data points with + for the positive examples
%   and o for the negative examples. X is assumed to be a Mx2 matrix.

% 要开始绘制图片了,应该说,用matlab来绘制图片感觉和直接用python来绘制图片难度差不太多

% Create New Figure
figure; hold on;

% ====================== YOUR CODE HERE ======================
% Instructions: Plot the positive and negative examples on a
%               2D plot, using the option 'k+' for the positive
%               examples and 'ko' for the negative examples.
%

% y应该是label
pos = find(y == 1); 
neg = find(y == 0);

% 然后开始绘制图片X(pos, 1)获得pos行的第一列数据,而X(pos, 2)获得pos行的第二列数据
plot(X(pos, 1), X(pos, 2), 'k+', 'LineWidth', 2, 'MarkerSize', 7);
plot(X(neg, 1), X(neg, 2), 'ko', 'MarkerFaceColor', 'y','MarkerSize', 7)







% =========================================================================



hold off;

end
